# - Try to find the SELinux library
# Once done this will define
#
#  SELINUX_FOUND - System has selinux
#  SELINUX_INCLUDE_DIR - The selinux include directory
#  SELINUX_LIBRARIES - The libraries needed to use selinux
#  SELINUX_DEFINITIONS - Compiler switches required for using selinux

#=============================================================================
# Copyright 2010  Michael Leupold <lemma@confuego.org>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

IF (SELINUX_INCLUDE_DIR AND SELINUX_LIBRARIES)
	# in cache already
	SET (SELinux_FIND_QUIETLY TRUE)
ENDIF (SELINUX_INCLUDE_DIR AND SELINUX_LIBRARIES)

# TODO: This should be restricted to Linux only
IF (NOT WIN32)
	# use pkg-config to get the directories
	FIND_PACKAGE (PkgConfig)
	PKG_CHECK_MODULES (PC_SELINUX libselinux)
	SET (SELINUX_DEFINITIONS ${PC_SELINUX_CFLAGS_OTHER})

	FIND_PATH (SELINUX_INCLUDE_DIR selinux/selinux.h
		HINTS
		${PC_SELINUX_INCLUDEDIR}
		${PC_SELINUX_INCLUDE_DIRS}
	)

	FIND_LIBRARY (SELINUX_LIBRARIES selinux libselinux
		HINTS
		${PC_SELINUX_LIBDIR}
		${PC_SELINUX_LIBRARY_DIRS}
	)

	INCLUDE (FindPackageHandleStandardArgs)

	# handle the QUIETLY and REQUIRED arguments and set SELINUX_FOUND
	# to TRUE if all listed variables are TRUE
	FIND_PACKAGE_HANDLE_STANDARD_ARGS (SELinux DEFAULT_MSG
		SELINUX_INCLUDE_DIR
		SELINUX_LIBRARIES
	)

	MARK_AS_ADVANCED (SELINUX_INCLUDE_DIR SELINUX_LIBRARIES)

ENDIF (NOT WIN32)
