# - Try to find the sndfile library
# Once done this will define
#
#  Sndfile_FOUND - system has sndfile
#  Sndfile_INCLUDE_DIRS - the sndfile include directories
#  Sndfile_LIBRARIES - Link these to use sndfile

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
  if (Sndfile_FIND_VERSION)
    set(version_string ">=${Sndfile_FIND_VERSION}")
  endif()
  pkg_check_modules(PC_SNDFILE sndfile${version_string})
else()
  # assume it was found
  set(PC_SNDFILE_FOUND TRUE)
endif()

if (PC_SNDFILE_FOUND)
  find_path(Sndfile_INCLUDE_DIRS sndfile.h
    HINTS ${PC_SNDFILE_INCLUDE_DIRS}
  )

  find_library(Sndfile_LIBRARIES NAMES sndfile
    HINTS ${PC_SNDFILE_LIBRARY_DIRS}
  )

  set(Sndfile_VERSION "${PC_SNDFILE_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Sndfile
                                  REQUIRED_VARS Sndfile_LIBRARIES Sndfile_INCLUDE_DIRS
                                  VERSION_VAR Sndfile_VERSION
)

mark_as_advanced(Sndfile_INCLUDE_DIRS Sndfile_LIBRARIES)
