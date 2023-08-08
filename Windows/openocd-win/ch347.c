/***************************************************************************                                                                     *
 *   Driver for CH347-JTAG interface V1.0                                  *
 *    Normal                                                                     *
 *   Copyright (C) 2022 Nanjing Qinheng Microelectronics Co., Ltd.         *
 *   Web: http://wch.cn                                                    *
 *   Author: WCH@TECH53 <tech@wch.cn>                                      *
 *                                                                         *
 *   CH347 is a high-speed USB bus converter chip that provides UART, I2C  *
 *   and SPI synchronous serial ports and JTAG interface through USB bus.  * 
 *                                                                         *
 *   The Jtag interface by CH347 can supports transmission frequency       *
 *   configuration up to 60MHz.                                            *
 *                                                                         *
 *   The USB2.0 to JTAG scheme based on CH347 can be used to build         *
 *   customized USB high-speed JTAG debugger and other products.           *                                                          
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         * 
 *            _____________                                                *
 *           |             |                                               *
 *      USB__|    CH347T   |____JTAG (TDO,TDI,TMS,TCK)                     *
 *           |_____________|                                               *
 *            __|__________                                                *
 *           |             |                                               *
 *           | 8 MHz XTAL  |                                               *
 *           |_____________|                                               *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if IS_CYGWIN == 1
#include "windows.h"
#undef LOG_ERROR
#endif

/* project specific includes */
#include <jtag/interface.h>
#include <jtag/commands.h>
#include <helper/time_support.h>
#include <helper/replacements.h>

/* system includes */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#define JTAGIO_STA_OUT_TDI (0x10)
#define JTAGIO_STA_OUT_TMS (0x02)
#define JTAGIO_STA_OUT_TCK (0x01)

#define TDI_H JTAGIO_STA_OUT_TDI
#define TDI_L 0
#define TMS_H JTAGIO_STA_OUT_TMS
#define TMS_L 0
#define TCK_H JTAGIO_STA_OUT_TCK
#define TCK_L 0

#define KHZ(n) ((n)*UINT64_C(1000))
#define MHZ(n) ((n)*UINT64_C(1000000))
#define GHZ(n) ((n)*UINT64_C(1000000000))

#define HW_TDO_BUF_SIZE              4096
#define SF_PACKET_BUF_SIZE           51200  // Command packet length
#define UCMDPKT_DATA_MAX_BYTES_USBHS 507    // The data length contained in each command packet during USB high-speed operation
#define USBC_PACKET_USBHS 512               // Maximum data length per packet at USB high speed
#define USBC_PACKET_USBHS_SINGLE 510        // Maximum length of a single packet at USB high speed
#define CH347_CMD_HEADER 3                  //JTAG Protocol header length

// Protocol transmission format: CMD (1 byte)+Length (2 bytes)+Data
#define CH347_CMD_INFO_RD 0xCA              // Parameter acquisition, used to obtain firmware version, JTAG interface related parameters, etc
#define CH347_CMD_JTAG_INIT 0xD0            // JTAG Interface Initialization Command
#define CH347_CMD_JTAG_BIT_OP 0xD1          // JTAG interface pin bit control command
#define CH347_CMD_JTAG_BIT_OP_RD 0xD2       // JTAG interface pin bit control and read commands
#define CH347_CMD_JTAG_DATA_SHIFT 0xD3      // JTAG interface data shift command
#define CH347_CMD_JTAG_DATA_SHIFT_RD 0xD4   // JTAG interface data shift and read command

#pragma pack(1)

typedef struct _CH347_info { // Record the CH347 pin status
    int TMS;
    int TDI;
    int TCK;

    int buffer_idx;
    UCHAR buffer[SF_PACKET_BUF_SIZE];

    int len_idx;
    int len_value;
    uint8_t lastCmd;
} _CH347_Info;

#pragma pack()

#ifdef _WIN32
#include <windows.h>
typedef int(__stdcall *pCH347OpenDevice)(unsigned long iIndex);
typedef int(__stdcall *pCH347CloseDevice)(unsigned long iIndex);
typedef unsigned long(__stdcall *pCH347SetTimeout)(unsigned long iIndex,        // Specify equipment serial number
                                                   unsigned long iWriteTimeout, // Specifies the timeout period for USB write out data blocks, in milliseconds mS, and 0xFFFFFFFF specifies no timeout (default)
                                                   unsigned long iReadTimeout); // Specifies the timeout period for USB reading data blocks, in milliseconds mS, and 0xFFFFFFFF specifies no timeout (default)
typedef unsigned long(__stdcall *pCH347WriteData)(unsigned long iIndex,         // Specify equipment serial number
                                                  void *oBuffer,                // Point to a buffer large enough to hold the descriptor
                                                  unsigned long *ioLength);     // Pointing to the length unit, the input is the length to be read, and the return is the actual read length
typedef unsigned long(__stdcall *pCH347ReadData)(unsigned long iIndex,          // Specify equipment serial number
                                                 void *oBuffer,                 // Point to a buffer large enough to hold the descriptor
                                                 unsigned long *ioLength);      // Pointing to the length unit, the input is the length to be read, and the return is the actual read length
typedef unsigned long(__stdcall *pCH347Jtag_INIT)(unsigned long iIndex,         // Specify equipment serial number
                                                  unsigned char iClockRate);    // Pointing to the length unit, the input is the length to be read, and the return is the actual read length
HMODULE uhModule;
BOOL ugOpen;
unsigned long ugIndex;
pCH347OpenDevice CH347OpenDevice;
pCH347CloseDevice CH347CloseDevice;
pCH347SetTimeout CH347SetTimeout;
pCH347ReadData CH347ReadData;
pCH347WriteData CH347WriteData;
pCH347Jtag_INIT CH347Jtag_INIT;
#elif defined(__linux__) 

#include <jtag/drivers/libusb_helper.h>

#define CH347_EPOUT 0x06u
#define CH347_EPIN  0x86u

bool ugOpen;
unsigned long ugIndex = 0;
struct libusb_device_handle *ch347_handle;

static const uint16_t ch347_vids[] = {0x1a86, 0};
static const uint16_t ch347_pids[] = {0x55dd, 0};

static uint32_t CH347OpenDevice(uint64_t iIndex)
{
    if (jtag_libusb_open(ch347_vids, ch347_pids, &ch347_handle, NULL) != ERROR_OK) {
        return false;
    } else {
        return true;
    }
}

static bool CH347WriteData(uint64_t iIndex, uint8_t *data, uint64_t *length)
{
	int ret, tmp = 0;
	ret = jtag_libusb_bulk_write(ch347_handle,
						CH347_EPOUT,
						(char *)data,
						*length,
						100, &tmp);
	*length = tmp;

   	if (!ret) {
   		return true;
   	} else {
   		return false;
   	}
}

static bool CH347ReadData(uint64_t iIndex, uint8_t *data, uint64_t *length)
{
    int ret, tmp = 0;

    int size = *length;
    ret = jtag_libusb_bulk_read(ch347_handle,
                                CH347_EPIN,
                                (char *)data,
                                size,
                                100, &tmp);

    *length = tmp;
   if (!ret) {
   		return true;
   } else {
   		return false;
   }
}

static bool CH347Jtag_INIT(uint64_t iIndex, uint8_t iClockRate)
{
    if (iClockRate > 5)
        return false;

    uint64_t i = 0;
    bool retVal;
    uint8_t cmdBuf[128] = "";
    cmdBuf[i++] = CH347_CMD_JTAG_INIT;
    cmdBuf[i++] = 6;
    cmdBuf[i++] = 0;

    cmdBuf[i++] = 0;          
    cmdBuf[i++] = iClockRate; 
    i += 4;                  

    uint64_t mLength = i;
    if (!CH347WriteData(iIndex, cmdBuf, &mLength) || (mLength != i)) {
        return false;
    }

    mLength = 4;
    memset(cmdBuf, 0, sizeof(cmdBuf));

    if (!CH347ReadData(iIndex, cmdBuf, &mLength) || (mLength != 4)) {
        return false;
    }

    retVal = ((cmdBuf[0] == CH347_CMD_JTAG_INIT) && (cmdBuf[CH347_CMD_HEADER] == 0));
    return retVal;
}

static bool CH347CloseDevice(uint64_t iIndex)
{
    jtag_libusb_close(ch347_handle);
    return true;
}

#endif 

int DevIsOpened;  // Whether the device is turned on
bool UsbHighDev = true;
unsigned long USBC_PACKET;

_CH347_Info ch347 = {0, 0, 0, 0, "", 0, 0, 0}; 

/**
 *  HexToString - Hex Conversion String Function
 *  @param buf    Point to a buffer to place the Hex data to be converted
 *  @param size   Pointing to the length unit where data needs to be converted
 *
 *  @return 	  Returns a converted string
 */
static char *HexToString(uint8_t *buf, uint32_t size)
{
    uint32_t i;
    char *str = calloc(size * 2 + 1, 1);

    for (i = 0; i < size; i++)
        sprintf(str + 2 * i, "%02x ", buf[i]);
    return str;
}

/**
 *  CH347_Write - CH347 Write
 *  @param oBuffer    Point to a buffer to place the data to be written out
 *  @param ioLength   Pointing to the length unit, the input is the length to be written out, and the return is the actual written length
 *
 *  @return 		  Write success returns 1, failure returns 0
 */
static int CH347_Write(void *oBuffer, unsigned long *ioLength)
{
    int ret = -1;
    unsigned long wlength = *ioLength, WI;

    if (*ioLength >= HW_TDO_BUF_SIZE)
        wlength = HW_TDO_BUF_SIZE;
    WI = 0;
    while (1) {
        ret = CH347WriteData(ugIndex, (uint8_t *)oBuffer + WI, &wlength);     
        LOG_DEBUG_IO("(size=%lu, buf=[%s]) -> %" PRIu32, wlength, HexToString((uint8_t *)oBuffer, wlength), (uint32_t)wlength);
        WI += wlength;
        if (WI >= *ioLength)
            break;
        if ((*ioLength - WI) > HW_TDO_BUF_SIZE)
            wlength = HW_TDO_BUF_SIZE;
        else
            wlength = *ioLength - WI;
    }

    *ioLength = WI;
    return ret;
}

/**
 * CH347_Read - CH347 Read
 * @param oBuffer  	Point to a buffer to place the data to be read in
 * @param ioLength 	Pointing to the length unit, the input is the length to be read, and the return is the actual read length
 *
 * @return 			Write success returns 1, failure returns 0
 */
static int CH347_Read(void *oBuffer, unsigned long *ioLength)
{
    unsigned long rlength = *ioLength;
    if (rlength > HW_TDO_BUF_SIZE)
        rlength = HW_TDO_BUF_SIZE;
    
    if (!CH347ReadData(ugIndex, oBuffer, &rlength)) 
    {
        LOG_ERROR("CH347_Read read data failure.");
        return false;
    }

    LOG_DEBUG_IO("(size=%lu, buf=[%s]) -> %" PRIu32, rlength, HexToString((uint8_t *)oBuffer, rlength), (uint32_t)rlength);
    *ioLength = rlength;
    return true;
}

static int CH347_available_buffer(void)
{
    return (SF_PACKET_BUF_SIZE - ch347.buffer_idx);
}

static void CH347_Flush_Buffer(void)
{
    unsigned long retlen = ch347.buffer_idx;
	int nb = ch347.buffer_idx, ret = ERROR_OK;

	while (ret == ERROR_OK && nb > 0) {
		ret = CH347_Write(ch347.buffer, &retlen);
		nb -= retlen;
	}
    memset(&ch347.buffer, 0, sizeof(ch347.buffer));
	ch347.buffer_idx = 0;
}

static void CH347_In_Buffer(uint8_t byte)
{
    if (CH347_available_buffer() < 1)
        CH347_Flush_Buffer();
    ch347.buffer[ch347.buffer_idx] = byte;
	ch347.buffer_idx++;
    if (CH347_available_buffer() == 0)
        CH347_Flush_Buffer();
    LOG_DEBUG("byte: %02x", byte);
}

static void CH347_In_Buffer_bytes(uint8_t *bytes, unsigned long bytes_length)
{
	if ((ch347.buffer_idx + bytes_length) > SF_PACKET_BUF_SIZE )
		CH347_Flush_Buffer();
    memcpy(&ch347.buffer[ch347.buffer_idx], bytes, bytes_length);
	ch347.buffer_idx += bytes_length;
    if (CH347_available_buffer() < 1)
        CH347_Flush_Buffer();
}

/**
 * CH347_ClockTms - Function function used to change the TMS value at the rising edge of TCK to switch its Tap state
 * @param BitBangPkt 	Protocol package
 * @param tms 		 	TMS value to be changed
 * @param BI		 	Protocol packet length
 *
 * @return 			 	Return protocol packet length
 */
static unsigned long CH347_ClockTms(unsigned char *BitBangPkt, int tms, unsigned long BI)
{
    uint8_t data = 0;
    unsigned char cmd = 0;

    if (tms == 1)
        cmd = TMS_H;
    else
        cmd = TMS_L;

    BI += 2;

    data = cmd | TDI_L | TCK_L;
    CH347_In_Buffer(data);
    data = cmd | TDI_L | TCK_H;
    CH347_In_Buffer(data);

    ch347.TMS = cmd;
    ch347.TDI = TDI_L;
    ch347.TCK = TCK_H;

    return BI;
}

/**
 * CH347_IdleClock - Function function to ensure that the clock is in a low state
 * @param BitBangPkt 	 Protocol package
 * @param BI  		 	 Protocol packet length
 *
 * @return 			 	 Return protocol packet length
 */
static unsigned long CH347_IdleClock(unsigned char *BitBangPkt, unsigned long BI)
{
    unsigned char byte = 0;
    byte |= ch347.TMS ? TMS_H : TMS_L;
    byte |= ch347.TDI ? TDI_H : TDI_L;
    BI++;
    CH347_In_Buffer(byte);

    return BI;
}

/**
 * CH347_TmsChange - Function function that performs state switching by changing the value of TMS
 * @param tmsValue 		 The TMS values that need to be switched form one byte of data in the switching order
 * @param step 	   		 The number of bit values that need to be read from the tmsValue value
 * @param skip 	   		 Count from the skip bit of tmsValue to step
 *
 */
static void CH347_TmsChange(const unsigned char *tmsValue, int step, int skip)
{
    int i;
    int index = ch347.buffer_idx;
    unsigned long BI, retlen, cmdLen; 
    unsigned char BitBangPkt[4096] = "";

    BI = CH347_CMD_HEADER;
    retlen = CH347_CMD_HEADER;
    LOG_DEBUG_IO("(TMS Value: %02x..., step = %d, skip = %d)", tmsValue[0], step, skip);

    for (i = 0; i < 3; i++)
        CH347_In_Buffer(0);

    for (i = skip; i < step; i++) {
        retlen = CH347_ClockTms(BitBangPkt, (tmsValue[i / 8] >> (i % 8)) & 0x01, BI);
        BI = retlen;
    }
    cmdLen = CH347_IdleClock(BitBangPkt, BI);
    ch347.buffer[index] =  CH347_CMD_JTAG_BIT_OP;
    ch347.buffer[index+1] =  (unsigned char)cmdLen - CH347_CMD_HEADER;
}

/**
 * CH347_TMS - By ch347_ execute_ Queue call
 * @param cmd 	   Upper layer transfer command parameters
 *
 */
static void CH347_TMS(struct tms_command *cmd)
{
    LOG_DEBUG_IO("(step: %d)", cmd->num_bits);
    CH347_TmsChange(cmd->bits, cmd->num_bits, 0);
}

/**
 * ch347_reset - ch347 reset Tap Status Function
 * @brief 	If there are more than six consecutive TCKs and TMS is high, the state machine can be set to a Test-Logic-Reset state
 *
 */
static int CH347_Reset(void)
{
    int index = ch347.buffer_idx;
    uint8_t data;
    unsigned char BI, i;

    for (i = 0; i < 3; i++) 
        CH347_In_Buffer(0);

    BI = CH347_CMD_HEADER;
    for (i = 0; i < 7; i++) {
        data = TMS_H | TDI_L | TCK_L;
        CH347_In_Buffer(data);
		data = TMS_H | TDI_L | TCK_H;
        CH347_In_Buffer(data);
        BI += 2;
    }
    data = TMS_H | TDI_L | TCK_L;
    CH347_In_Buffer(data);
    BI++;

    ch347.buffer[index] = CH347_CMD_JTAG_BIT_OP;
    ch347.buffer[index+1] = BI-CH347_CMD_HEADER;

    ch347.TMS = TMS_L;
    ch347.TDI = TDI_H;
    ch347.TCK = TCK_L;
	CH347_Flush_Buffer();
    return true;
}

/**
 * CH347_MovePath - Obtain the current Tap status and switch to the status TMS value passed down by cmd
 * @param cmd Upper layer transfer command parameters
 *
 */
static void CH347_MovePath(struct pathmove_command *cmd)
{
    int i;
    int index = ch347.buffer_idx;
    unsigned long BI, retlen = 0, cmdLen;
    unsigned char BitBangPkt[4096] = "";

    BI = CH347_CMD_HEADER;

    for (i = 0; i < 3; i++)
        CH347_In_Buffer(0);
    LOG_DEBUG_IO("(num_states=%d, last_state=%d)",
                 cmd->num_states, cmd->path[cmd->num_states - 1]);

    for (i = 0; i < cmd->num_states; i++) {
        if (tap_state_transition(tap_get_state(), false) == cmd->path[i])
            retlen = CH347_ClockTms(BitBangPkt, 0, BI);
        BI = retlen;
        if (tap_state_transition(tap_get_state(), true) == cmd->path[i])
            retlen = CH347_ClockTms(BitBangPkt, 1, BI);
        BI = retlen;
        tap_set_state(cmd->path[i]);
    }

    cmdLen = CH347_IdleClock(BitBangPkt, BI);
    
    ch347.buffer[index] = CH347_CMD_JTAG_BIT_OP;
    ch347.buffer[index+1] = (unsigned char)cmdLen - CH347_CMD_HEADER;
}

/**
 * CH347_MoveState - Toggle the Tap state to the Target state stat
 * @param stat Pre switch target path
 * @param skip Number of digits to skip
 *
 */
static void CH347_MoveState(tap_state_t state, int skip)
{
    uint8_t tms_scan;
    int tms_len;

    LOG_DEBUG_IO("(from %s to %s)", tap_state_name(tap_get_state()),
                 tap_state_name(state));
    if (tap_get_state() == state)
        return;
    tms_scan = tap_get_tms_path(tap_get_state(), state);
    tms_len = tap_get_tms_path_len(tap_get_state(), state);
    CH347_TmsChange(&tms_scan, tms_len, skip);
    tap_set_state(state);
}

/**
 * CH347_WriteRead - CH347 Batch read/write function
 * @param bits 			 Read and write data this time
 * @param nb_bits 		 Incoming data length
 * @param scan			 The transmission method of incoming data to determine whether to perform data reading
 *
 */
static void CH347_WriteRead(uint8_t *bits, int nb_bits, enum scan_type scan)
{
    int nb8 = nb_bits / 8;
    int nb1 = nb_bits % 8;
	int i;
    bool IsRead = false;
	uint8_t TMS_Bit = 0, TDI_Bit = 0, ret;
    uint8_t *tdos = calloc(1, nb_bits / 8 + 32);
    static uint8_t BitBangPkt[SF_PACKET_BUF_SIZE];
    static uint8_t byte0[SF_PACKET_BUF_SIZE];
    unsigned char temp[512] = "";
    unsigned char temp_a[512] = "";
    unsigned long BI = 0, TxLen, RxLen, DI, DII, PktDataLen, DLen = 0,tempIndex;  

    // The last TDI bit will be output in bit band mode, with nb1 ensuring it is not 0, allowing it to output the last 1 bit of data when TMS changes
    if (nb8 > 0 && nb1 == 0) {
        nb8--;
        nb1 = 8;
    }

    IsRead = (scan == SCAN_IN || scan == SCAN_IO);
    DI = BI = 0;
    while (DI < (unsigned long)nb8) {
        // Build Data Package
        if ((nb8 - DI) > UCMDPKT_DATA_MAX_BYTES_USBHS)
            PktDataLen = UCMDPKT_DATA_MAX_BYTES_USBHS;
        else
            PktDataLen = nb8 - DI;

        DII = PktDataLen;

        if (IsRead)
            CH347_In_Buffer(CH347_CMD_JTAG_DATA_SHIFT_RD);
        else
            CH347_In_Buffer(CH347_CMD_JTAG_DATA_SHIFT);

        CH347_In_Buffer((uint8_t)(PktDataLen >> 0)&0xFF);
		CH347_In_Buffer((uint8_t)(PktDataLen >> 8)&0xFF);

		if (bits)
			CH347_In_Buffer_bytes(&bits[DI], PktDataLen);
		else 
			CH347_In_Buffer_bytes(byte0, PktDataLen);
        // If data needs to be read back, determine the current BI value and issue the command
        if (IsRead) {
			CH347_Flush_Buffer();
            BI = 0;

            ret = ERROR_OK;
            while (ret == ERROR_OK && PktDataLen > 0) {
                RxLen = PktDataLen + CH347_CMD_HEADER;
                TxLen = RxLen;
                if (!(ret = CH347_Read(temp, &RxLen))) {
                    LOG_ERROR("CH347_WriteRead read usb data failure.\n");
                    return;
                }

                if (RxLen != TxLen) {
                    if (!(ret = CH347_Read(temp_a, &TxLen))) {
                        LOG_ERROR("CH347_WriteRead read usb data failure.\n");
                        return;
                    }
                    memcpy(&temp[RxLen], temp_a, TxLen);
                    RxLen += TxLen;
                }

                if (RxLen != 0)
                    memcpy(&tdos[DI], &temp[CH347_CMD_HEADER], (RxLen - CH347_CMD_HEADER));
                PktDataLen -= RxLen;
            }
        }

        DI += DII;

    }

    // Build a command package that outputs the last 1 bit of TDI data
    if (bits) {
        CH347_In_Buffer(IsRead?CH347_CMD_JTAG_BIT_OP_RD:CH347_CMD_JTAG_BIT_OP);
        DLen = (nb1 * 2) + 1;
        CH347_In_Buffer((uint8_t)(DLen>>0)&0xFF);
		CH347_In_Buffer((uint8_t)(DLen>>8)&0xFF);
        TMS_Bit = TMS_L;
        for (i = 0; i < nb1; i++) {
            if ((bits[nb8] >> i) & 0x01)
                TDI_Bit = TDI_H;
            else
                TDI_Bit = TDI_L;

            if ((i + 1) == nb1) //The last bit is output in Exit1-DR state
                TMS_Bit = TMS_H;
            CH347_In_Buffer(TMS_Bit | TDI_Bit | TCK_L);
			CH347_In_Buffer(TMS_Bit | TDI_Bit | TCK_H);
        }
        CH347_In_Buffer(TMS_Bit | TDI_Bit | TCK_L);
    }

    ch347.TMS = TMS_Bit;
    ch347.TDI = TDI_Bit;
    ch347.TCK = TCK_L;

    // Read the last byte of data in Bit-Bang mode
    if (nb1 && IsRead) {
        TxLen = DLen + CH347_CMD_HEADER;
        CH347_Flush_Buffer();
        BI = 0;
        RxLen = TxLen;
        if (!(ret = CH347_Read(temp, &RxLen))) {
            LOG_ERROR("CH347_WriteRead read usb data failure.");
            return;
        }
        if ((RxLen) < ((TxLen+CH347_CMD_HEADER))/2)
		{
			if (!(ret = CH347_Read(temp_a, &TxLen))) 
			{
				LOG_ERROR("CH347_WriteRead read usb data failure.");
			} 
			memcpy(&temp[RxLen], temp_a, TxLen);
			RxLen += TxLen;
		}

        for (i = 0; ret == true && i < nb1; i++) {
            if (temp[CH347_CMD_HEADER + i] & 1)
                tdos[nb8] |= (1 << i);
            else
                tdos[nb8] &= ~(1 << i);
        }
    }

    if (bits) {
        memcpy(bits, tdos, DIV_ROUND_UP(nb_bits, 8));
    }

    free(tdos);
    LOG_DEBUG_IO("bits %d str value: [%s].\n", DIV_ROUND_UP(nb_bits, 8), HexToString(bits, DIV_ROUND_UP(nb_bits, 8)));

    // Pull down TCK and TDI to low level, as TDI sampling occurs on the rising edge of TCK. If the state does not change, TDI sampling may occur on the falling edge of TCK
    tempIndex = ch347.buffer_idx;
	for(i = 0; i < CH347_CMD_HEADER; i++) 
		CH347_In_Buffer(0);
    BI = CH347_CMD_HEADER;
    BI = CH347_IdleClock(BitBangPkt, BI);

	ch347.buffer[tempIndex] = CH347_CMD_JTAG_BIT_OP;
	ch347.buffer[tempIndex+1] = (unsigned char)BI - CH347_CMD_HEADER;
    TxLen = BI;
}

static void CH347_RunTest(int cycles, tap_state_t state)
{
    LOG_DEBUG_IO("%s(cycles=%i, end_state=%d)", __func__, cycles, state);
    if (tap_get_state() != TAP_IDLE)
		CH347_MoveState(TAP_IDLE, 0);

    uint8_t tmsValue = 0;
    CH347_TmsChange(&tmsValue, 7, 1);

    CH347_WriteRead(NULL, cycles, SCAN_OUT);
    CH347_MoveState(state, 0);
}

static void CH347_TableClocks(int cycles)
{
    LOG_DEBUG_IO("%s(cycles=%i)", __func__, cycles);
    CH347_WriteRead(NULL, cycles, SCAN_OUT);
}

/**
 * CH347_Scan - Switch to SHIFT-DR or SHIFT-IR status for scanning
 * @param cmd 	    Upper layer transfer command parameters
 *
 * @return 	        Success returns ERROR_OK
 */
static int CH347_Scan(struct scan_command *cmd)
{
    int scan_bits;
    uint8_t *buf = NULL;
    enum scan_type type;
    int ret = ERROR_OK;
    static const char *const type2str[] = { "", "SCAN_IN", "SCAN_OUT", "SCAN_IO"};
    char *log_buf = NULL;

    type = jtag_scan_type(cmd);
    scan_bits = jtag_build_buffer(cmd, &buf);

    if (cmd->ir_scan)
        CH347_MoveState(TAP_IRSHIFT, 0);
    else
        CH347_MoveState(TAP_DRSHIFT, 0);

    log_buf = HexToString(buf, DIV_ROUND_UP(scan_bits, 8));
    LOG_DEBUG_IO("Scan");
    LOG_DEBUG_IO("%s(scan=%s, type=%s, bits=%d, buf=[%s], end_state=%d)", __func__,
                 cmd->ir_scan ? "IRSCAN" : "DRSCAN",
                 type2str[type],
                 scan_bits, log_buf, cmd->end_state);

    free(log_buf);

    CH347_WriteRead(buf, scan_bits, type);

    ret = jtag_read_buffer(buf, cmd);
    free(buf);

    CH347_MoveState(cmd->end_state, 1);

    return ret;
}

static void CH347_Sleep(int us)
{
    LOG_DEBUG_IO("%s(us=%d)", __func__, us);
    jtag_sleep(us);
}

static int ch347_execute_queue(void)
{
    struct jtag_command *cmd;
    static int first_call = 1;
    int ret = ERROR_OK;

    if (first_call) {
        first_call--;
        CH347_Reset();
    }

    for (cmd = jtag_command_queue; ret == ERROR_OK && cmd;
         cmd = cmd->next) {
        switch (cmd->type) {
        case JTAG_RESET:
            CH347_Reset();
            break;
        case JTAG_RUNTEST:
            CH347_RunTest(cmd->cmd.runtest->num_cycles,
                          cmd->cmd.runtest->end_state);
            break;
        case JTAG_STABLECLOCKS:
            CH347_TableClocks(cmd->cmd.stableclocks->num_cycles);
            break;
        case JTAG_TLR_RESET:
            CH347_MoveState(cmd->cmd.statemove->end_state, 0);
            break;
        case JTAG_PATHMOVE:
            CH347_MovePath(cmd->cmd.pathmove);
            break;
        case JTAG_TMS:
            CH347_TMS(cmd->cmd.tms);
            break;
        case JTAG_SLEEP:
            CH347_Sleep(cmd->cmd.sleep->us);
            break;
        case JTAG_SCAN:
            ret = CH347_Scan(cmd->cmd.scan);
            break;
        default:
            LOG_ERROR("BUG: unknown JTAG command type 0x%X",
                      cmd->type);
            ret = ERROR_FAIL;
            break;
        }
    }
	CH347_Flush_Buffer();    
    return ret;
}

/**
 * ch347_init - CH347 Initialization function
 *
 *  Todo:
 *                Initialize dynamic library functions
 *                Open Device
 *  @return 	  Success returns 0, failure returns ERROR_FAIL
 */
static int ch347_init(void)
{
#ifdef _WIN32
    if (uhModule == 0) {
        uhModule = LoadLibrary("CH347DLL.DLL");
        if (uhModule) {
            CH347OpenDevice = (pCH347OpenDevice)GetProcAddress(uhModule, "CH347OpenDevice");
            CH347CloseDevice = (pCH347CloseDevice)GetProcAddress(uhModule, "CH347CloseDevice");
            CH347ReadData = (pCH347ReadData)GetProcAddress(uhModule, "CH347ReadData");
            CH347WriteData = (pCH347WriteData)GetProcAddress(uhModule, "CH347WriteData");
            CH347SetTimeout = (pCH347SetTimeout)GetProcAddress(uhModule, "CH347SetTimeout");
            CH347Jtag_INIT = (pCH347Jtag_INIT)GetProcAddress(uhModule, "CH347Jtag_INIT");
            if (CH347OpenDevice == NULL || CH347CloseDevice == NULL || CH347SetTimeout == NULL || CH347ReadData == NULL || CH347WriteData == NULL || CH347Jtag_INIT == NULL) {
                LOG_ERROR("Jtag_init error ");
                return ERROR_FAIL;
            }
        }
    }
    DevIsOpened = CH347OpenDevice(ugIndex);
#elif defined(__linux__)
    DevIsOpened = CH347OpenDevice(ugIndex);
    ugIndex = DevIsOpened;
#endif
    if (!DevIsOpened) {
        LOG_ERROR("CH347 Open Error.");
        return ERROR_FAIL;
    }

    USBC_PACKET = USBC_PACKET_USBHS; // The default is USB2.0 high-speed, with a single transfer USB packet size of 512 bytes

    tap_set_state(TAP_RESET);
    return 0;
}

/**
 * ch347_quit - CH347 Device Release Function
 *
 * Todo:
 *              Reset JTAG pin signal
 *              Close
 *  @return 	always returns 0
 */
static int ch347_quit(void)
{
    // Set all signal lines to low level before exiting
    unsigned long retlen = 5;
    uint8_t byte[5] = {CH347_CMD_JTAG_BIT_OP, 0x01, 0x00, 0x00, 0x00};

    CH347_Write(byte, &retlen);

    if (DevIsOpened) {
        CH347CloseDevice(ugIndex);
        LOG_INFO("Close the CH347.");
        DevIsOpened = false;
    }
    return 0;
}

/**
 * ch347_speed - CH347 TCK frequency setting
 *  @param speed Frequency size set
 *  @return 	 Success returns ERROR_OK，failed returns FALSE
 */
static int ch347_speed(int speed)
{
    unsigned long i = 0;
    int retval = -1;
    int speed_clock[6] = {MHZ(1.875), MHZ(3.75), MHZ(7.5), MHZ(15), MHZ(30), MHZ(60)};

    for (i = 0; i < (sizeof(speed_clock) / sizeof(int)); i++) {
        if ((speed >= speed_clock[i]) && (speed <= speed_clock[i + 1])) {
            retval = CH347Jtag_INIT(ugIndex, i + 1);
            if (!retval) {
                LOG_ERROR("Couldn't set CH347 TCK speed");
                return retval;
            } else {
                break;
            }
        } else if (speed < speed_clock[0]) {
            retval = CH347Jtag_INIT(ugIndex, 0);
            if (!retval) {
                LOG_ERROR("Couldn't set CH347 TCK speed");
                return retval;
            } else {
                break;
            }
        }
    }

    return ERROR_OK;
}

static int ch347_speed_div(int speed, int *khz)
{
    *khz = speed / 1000;
    return ERROR_OK;
}

static int ch347_khz(int khz, int *jtag_speed)
{
    if (khz == 0) {
        LOG_ERROR("Couldn't support the adapter speed");
        return ERROR_FAIL;
    }
    *jtag_speed = khz * 1000;
    return ERROR_OK;
}

COMMAND_HANDLER(ch347_handle_vid_pid_command)
{
    return ERROR_OK;
}

static const struct command_registration ch347_subcommand_handlers[] = {
    {
        .name = "vid_pid",
        .handler = ch347_handle_vid_pid_command,
        .mode = COMMAND_CONFIG,
        .help = "",
        .usage = "",
    },
    COMMAND_REGISTRATION_DONE};

static const struct command_registration ch347_command_handlers[] = {
    {
        .name = "ch347",
        .mode = COMMAND_ANY,
        .help = "perform ch347 management",
        .chain = ch347_subcommand_handlers,
        .usage = "",
    },
    COMMAND_REGISTRATION_DONE};

static struct jtag_interface ch347_interface = {
    .supported = DEBUG_CAP_TMS_SEQ,
    .execute_queue = ch347_execute_queue,
};

struct adapter_driver ch347_adapter_driver = {
    .name = "ch347",
    .transports = jtag_only,
    .commands = ch347_command_handlers,

    .init = ch347_init,
    .quit = ch347_quit,
    .speed = ch347_speed,
    .khz = ch347_khz,
    .speed_div = ch347_speed_div,

    .jtag_ops = &ch347_interface,
};