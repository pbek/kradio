/***************************************************************************
                          recording-interfaces.cpp  -  description
                             -------------------
    begin                : Mi Aug 27 2003
    copyright            : (C) 2003 by Martin Witte
    email                : witte@kawo1.rwth-aachen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "recording-interfaces.h"
#include "recording-context.h"

// IRecording
 
IF_IMPL_SENDER  (  IRecording::notifyRecordingStarted(),
                   noticeRecordingStarted()                                )

IF_IMPL_SENDER  (  IRecording::notifyRecordingStopped(),
                   noticeRecordingStarted()                                )

IF_IMPL_SENDER  (  IRecording::notifyRecordingDirectoryChanged(const QString &s),
                   noticeRecordingDirectoryChanged(s)                      )
IF_IMPL_SENDER  (  IRecording::notifyRecordingContextChanged(const RecordingContext &s),
                   noticeRecordingContextChanged(s)                      )

// IRecordingClient

IF_IMPL_SENDER  (  IRecordingClient::sendStartRecording(),
                   startRecording()                                        )

IF_IMPL_SENDER  (  IRecordingClient::sendStopRecording(),
                   stopRecording()                                         )

IF_IMPL_SENDER  (  IRecordingClient::sendRecordingDirectory(const QString &s),
                   setRecordingDirectory(s)                                )

IF_IMPL_QUERY   (  bool IRecordingClient::queryIsRecording(),
                   isRecording(),
                   false                                                   )

IF_IMPL_QUERY   (  const QString &IRecordingClient::queryRecordingDirectory(),
                   getRecordingDirectory(),
                   QString::null                                           )

static const RecordingContext invalidContext;
IF_IMPL_QUERY   (  const RecordingContext &IRecordingClient::queryRecordingContext(),
                   getRecordingContext(),
                   invalidContext                                          )


void IRecordingClient::noticeConnected  (cmplInterface *, bool /*pointer_valid*/)
{
	if (queryIsRecording()) noticeRecordingStarted();
	else                    noticeRecordingStopped();
	noticeRecordingDirectoryChanged(queryRecordingDirectory());
	noticeRecordingContextChanged(queryRecordingContext());
}


void IRecordingClient::noticeDisconnected   (cmplInterface *, bool /*pointer_valid*/)
{
	if (queryIsRecording()) noticeRecordingStarted();
	else                    noticeRecordingStopped();
	noticeRecordingDirectoryChanged(queryRecordingDirectory());
	noticeRecordingContextChanged(queryRecordingContext());
}

