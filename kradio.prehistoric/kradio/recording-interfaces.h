/***************************************************************************
                          recording-interfaces.h  -  description
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

#ifndef KRADIO_RECORDING_INTERFACES_H
#define KRADIO_RECORDING_INTERFACES_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "interfaces.h"

struct RecordingContext;

///////////////////////////////////////////////////////////////////////

INTERFACE(IRecording, IRecordingClient)
{
public :
	IF_CON_DESTRUCTOR(IRecording, -1)

RECEIVERS:
	IF_RECEIVER(  startRecording()                                 )
	IF_RECEIVER(  stopRecording()                                  )
    IF_RECEIVER(  setRecordingDirectory(const QString &)           )

SENDERS:
	IF_SENDER  (  notifyRecordingStarted()                         )
	IF_SENDER  (  notifyRecordingStopped()                         )
    IF_SENDER  (  notifyRecordingDirectoryChanged(const QString &) )
	IF_SENDER  (  notifyRecordingContextChanged(const RecordingContext &c))

ANSWERS:
	IF_ANSWER  (  bool             isRecording() const             )
	IF_ANSWER  (  const QString   &getRecordingDirectory() const   )
	IF_ANSWER  (  const RecordingContext &getRecordingContext() const  )
};


INTERFACE(IRecordingClient, IRecording)
{
friend class IRecording;

public :
	IF_CON_DESTRUCTOR(IRecordingClient, 1)

SENDERS:
	IF_SENDER  (  sendStartRecording()                             )
	IF_SENDER  (  sendStopRecording()                              )
    IF_SENDER  (  sendRecordingDirectory(const QString &)          )

RECEIVERS:
	IF_RECEIVER(  noticeRecordingStarted()                         )
	IF_RECEIVER(  noticeRecordingStopped()                         )
    IF_RECEIVER(  noticeRecordingDirectoryChanged(const QString &) )
	IF_RECEIVER(  noticeRecordingContextChanged(const RecordingContext &c))

QUERIES:
	IF_QUERY   (  bool            queryIsRecording()               )
	IF_QUERY   (  const QString  &queryRecordingDirectory()        )
	IF_QUERY   (  const RecordingContext &queryRecordingContext()  )

RECEIVERS:
	virtual void noticeConnected    (cmplInterface *, bool pointer_valid);
	virtual void noticeDisconnected (cmplInterface *, bool pointer_valid);
};



#endif
