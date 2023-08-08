##
## This file is part of the libjaylink project.
##
## Copyright (C) 2016 Marc Schink <jaylink-dev@marcschink.de>
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.
##

# serial 20161011

## _JAYLINK_SET_PACKAGE_VERSION(prefix, version, major, minor, micro)
##
m4_define([_JAYLINK_SET_PACKAGE_VERSION], [
	m4_assert([$# == 5])

	# Get the Git repository description. Ensure that we use our Git
	# repository even when there is a parent Git repository.
	git_version=`git -C "$srcdir" --git-dir=.git describe --dirty 2> /dev/null`

	# If available, use the Git description as package version. Otherwise,
	# use the version provided by Autoconf.
	AS_IF([test -n "$git_version"], [version="$git_version"],
		[version="$2"])

	AC_SUBST([$1_MAJOR], [$3])
	AC_SUBST([$1_MINOR], [$4])
	AC_SUBST([$1_MICRO], [$5])
	AC_SUBST([$1], [$version])
])

## JAYLINK_SET_PACKAGE_VERSION(prefix, version)
##
## Parse the package version string of the format <major>.<minor>.<micro> and
## set the variables <prefix>_{MAJOR,MINOR,MICRO} to their corresponding
## values.
##
## Set the variable <prefix> to the package version string. If Git is
## available, append the short Git revision hash of the current commit to the
## version string if there is no release tag for the package version on it.
##
AC_DEFUN([JAYLINK_SET_PACKAGE_VERSION], [
	m4_assert([$# == 2])

	_JAYLINK_SET_PACKAGE_VERSION([$1], [$2],
		m4_unquote(m4_split(m4_expand([$2]), [\.])))
])

## _JAYLINK_SET_LIBRARY_VERSION(prefix, version, current, revision, age)
##
m4_define([_JAYLINK_SET_LIBRARY_VERSION], [
	m4_assert([$# == 5])

	AC_SUBST([$1_CURRENT], [$3])
	AC_SUBST([$1_REVISION], [$4])
	AC_SUBST([$1_AGE], [$5])
	AC_SUBST([$1], [$2])
])

## JAYLINK_SET_LIBRARY_VERSION(prefix, version)
##
## Parse the library version string of the format <current>:<revision>:<age>
## and set the variables <prefix>_{CURRENT,REVISION,AGE} to their corresponding
## values.
##
## Set the variable <prefix> to the library version string.
##
AC_DEFUN([JAYLINK_SET_LIBRARY_VERSION], [
	m4_assert([$# == 2])

	_JAYLINK_SET_LIBRARY_VERSION([$1], [$2],
		m4_unquote(m4_split([$2], [:])))
])
