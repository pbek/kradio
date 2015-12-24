# - Try to find the LIRC client library
# Once done this will define
#
#  LircClient_FOUND - system has LIRC client
#  LircClient_INCLUDE_DIRS - the LIRC client include directories
#  LircClient_LIBRARIES - Link these to use LIRC client

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
  if (LircClient_FIND_VERSION)
    set(version_string ">=${LircClient_FIND_VERSION}")
  endif()
  pkg_check_modules(PC_LIRC liblircclient0${version_string})
else()
  # assume it was found
  set(PC_LIRC_FOUND TRUE)
endif()

if (PC_LIRC_FOUND)
  find_path(LircClient_INCLUDE_DIRS lirc/lirc_client.h
    HINTS ${PC_LIRC_INCLUDE_DIRS}
  )

  find_library(LircClient_LIBRARIES NAMES lirc_client
    HINTS ${PC_LIRC_LIBRARY_DIRS}
  )

  set(LircClient_VERSION "${PC_LIRC_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LircClient
                                  REQUIRED_VARS LircClient_LIBRARIES LircClient_INCLUDE_DIRS
                                  VERSION_VAR LircClient_VERSION
)

mark_as_advanced(LircClient_INCLUDE_DIRS LircClient_LIBRARIES)
