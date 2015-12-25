# - Try to find the ALSA library
# Once done this will define
#
#  ALSA_FOUND - system has ALSA
#  ALSA_INCLUDE_DIRS - the ALSA include directories
#  ALSA_LIBRARIES - Link these to use ALSA

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
  if (ALSA_FIND_VERSION)
    set(version_string ">=${ALSA_FIND_VERSION}")
  endif()
  pkg_check_modules(PC_ALSA alsa${version_string})
  unset(version_string)
else()
  # assume it was found
  set(PC_ALSA_FOUND TRUE)
endif()

if (PC_ALSA_FOUND)
  find_path(ALSA_INCLUDE_DIRS alsa/asoundlib.h
    HINTS ${PC_ALSA_INCLUDE_DIRS}
  )

  find_library(ALSA_LIBRARIES NAMES asound
    HINTS ${PC_ALSA_LIBRARY_DIRS}
  )

  set(ALSA_VERSION "${PC_ALSA_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ALSA
                                  REQUIRED_VARS ALSA_LIBRARIES ALSA_INCLUDE_DIRS
                                  VERSION_VAR ALSA_VERSION
)

mark_as_advanced(ALSA_INCLUDE_DIRS ALSA_LIBRARIES)
