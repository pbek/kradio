# - Try to find the libmms library
# Once done this will define
#
#  Libmms_FOUND - system has libmms
#  Libmms_INCLUDE_DIRS - the libmms include directories
#  Libmms_LIBRARIES - Link these to use libmms

# Copyright (c) 2015, Pino Toscano <toscano.pino@tiscali.it>

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

find_package(PkgConfig)

if (PKG_CONFIG_FOUND)
  if (Libmms_FIND_VERSION)
    set(version_string ">=${Libmms_FIND_VERSION}")
  endif()
  pkg_check_modules(PC_LIBMMS libmms${version_string})
  unset(version_string)
else()
  # assume it was found
  set(PC_LIBMMS_FOUND TRUE)
endif()

if (PC_LIBMMS_FOUND)
  find_path(Libmms_INCLUDE_DIRS libmms/mmsx.h
    HINTS ${PC_LIBMMS_INCLUDE_DIRS}
  )

  find_library(Libmms_LIBRARIES NAMES mms
    HINTS ${PC_LIBMMS_LIBRARY_DIRS}
  )

  set(Libmms_VERSION "${PC_LIBMMS_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libmms
                                  REQUIRED_VARS Libmms_LIBRARIES Libmms_INCLUDE_DIRS
                                  VERSION_VAR Libmms_VERSION
)

mark_as_advanced(Libmms_INCLUDE_DIRS Libmms_LIBRARIES)
