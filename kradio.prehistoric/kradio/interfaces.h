/***************************************************************************
                          interfaces.h  -  description
                             -------------------
    begin                : Fre Feb 28 2003
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
 
#ifndef KRADIO_INTERFACES_H
#define KRADIO_INTERFACES_H

#include <qptrlist.h>
#include <kdebug.h>
#include <typeinfo>

/*
/////////////////////////////////////////////////////////////////////////////

   Interfaces - Our Concept

   Without connection management an interface can be defined easily as empty
   abstract C++-Class. But that's not what we want.

   Our interfaces also provide connection management. Thus each interface has
   exactly one matching counterpart, the complementary interface (cmplIF).
   Therefore connecting two objects that have matching interfaces can be
   automated.

   Our interfaces have to be able to support the following "functions":

   - send and receive messages (e.g. notifications, commands, ...) to
     all connected interfaces. These functions do not need a return value,
     but in some cases the sender might want to know if anyone has received
     his message. Thus a boolean return value should indicate if the message
     was handled or ignored.

   - query for information on connected interfaces / answer queries. These
     functions usually have a return value. A query is only executed on the
     "current" or - if not selected - the first or only connection.

/////////////////////////////////////////////////////////////////////////////

   Why are we not using QT signal/slots?

   First the idea of using qt for connecting interfaces is very nice, as the
   signal/slot model is well known and hopefully properly implemented. 

   But there are some problems:

   - Signals/slots do not support return values, except "call by reference".
     To provide queries or a delivery feedback for messages, wrapper functions
     would have been necessary.
     
   - Qt does not support multiple inheritance of QObjects. Thus even signals
     have to be declared abstract by the interface though the (later)
     implementation is already known.

     Those functions have to be declared as signals in the interface
     implementation (derived from QObject) though the implementation does not
     want to worry about these signals.
     
   - Qt does connect functions (signals/slots) and not interfaces. These
     functions have to be connected separately. By that it is possible to
     forget to connect signals/slots of that interfaces.

   - Aggregation of multiple interface implementations (each one is an QObject)
     is not possible because qt does not allow multiple inheritance of QObjects

/////////////////////////////////////////////////////////////////////////////

   What about our own solution?

   Well, it eliminates at least the qt-problems explained above. But first we
   need a common mechanism to manage interface connections. This functionality
   can be provided by a common base class "InterfaceBase". It stores all
   connected interfaces in a list of InterfaceBase pointers, e.g. QPtrList.

   With this approach we would have some problems:

   - When calling a function of a connected interface a slow dynamic_cast
     is necessary to upcast the stored InterfaceBase pointer to the
     apropriate type.

   - Multiple inheritance of InterfaceBase must not be virtual. Otherwise
     interface connection management is mixed between interfaces.
     (well, virtual inheritance is usually no real issue, but worth a hint;-)

   To avoid these problems, InterfaceBase is a template with two parameters,
   thisIF (IF = interface) and cmplIF (complementary IF). With that
   information the base class for an interface is capable to handle
   connections with the correct type information. Additionally some pseudo
   types are declared (thisInterface, cmplInterface, IFList, IFIterator) to
   make easy-to-use macros for messages and queries possible.

/////////////////////////////////////////////////////////////////////////////

   How do I use it ?   - Declarations

   First you have to declare the two matching interface-classes as unkown
   classes, because both their names are used in the class declarations.
   Afterwards you can declare both classes as class derived from
   InterfaceBase.

       class Interface;
       class ComplementaryInterface;

       class Interface : public InterfaceBase<Interface, ComplementaryInterface>
       {
       	...
       };

       class ComplementaryInterface : public InterfaceBase<ComplementaryInterface, Interface>
       {
       	...
       };

   With macro abbreviation:

       INTERFACE(Interface, ComplementaryInterface)
       {
       };

       INTERFACE(ComplementaryInterface, Interface)
       {
       };


   In order to receive/send Messages or query/answer queries we have to declare
   special methods:
   
   - sending Messages

     Declare a virtual constant method with return value "int" and the desired
     parameters. The return value will indicate how many receivers have handled
     the message:

         virtual bool  SendingMessages(int any_or_non_param) const;

     Abbreviation by macros:

     	 IF_SENDER(    SendingMessages(int any_or_non_param)   )


   - receiving Messages

     Declare an abstract Method with return value "bool", and the desired
     paramters. The return value indicates wether the message was handled or not:

         virtual bool  ReceivingMessages(int any_or_non_param) = 0;

     Abbreviation by macros:

     	 IF_RECEIVER(  ReceivingMessages(int any_or_non_param)   )


     The method has to be implemented by a derived class. The current item of the
     receivers conntions list is set to the sender.
     

   - querying queries

     Declare a virtual constant method with the desired return value and
     parameters:

     	 virtual int   QueryingQueries(int another_param) const;

     Abbreviation by macros:

         IF_QUERY(     int QueryingQueries(int another_param)        )


   - answering queries

     Declare an abstract Method with return value void, and the desired
     paramters:

         virtual void  AnsweringQueries(int another_param) = 0;

     Abbreviation by macros:

     	 IF_ANSWER(    AnsweringQueries(int another_param)   )

     The method has to be implemented by a derived class. The current item of the
     receivers conntions list is set to the sender.


   At last a note on maxConnections. This member is set on initialization by
   the constructor and thus can be set in a derived class in it's own
   constructor. Negative values are interpreted as "unlimited".


/////////////////////////////////////////////////////////////////////////////

   How do I use it ?   - Implementations

   Because we do not have a MOC as Qt does, we have to implement our sending
   or querying methods by hand. But this minor disadvantage should be
   considered as less important than the fact, that this implementation is
   done where it belongs to. Especially because there are easy to use macros
   to do this:

   int   ComplementaryInterface::SendingMessages(int any_or_non_param) const
   {
       IF_SEND_MESSAGE( ReceivingMessages(any_or_non_param)  )
       // macro includes "return #receivers"
   }

   int   ComplementaryInterface::QueryingQueries(int another_param) const
   {
       IF_SEND_QUERY( AnsweringQuery(another_param), (int)"default return value" )
   }


   Even shorter:

   IF_IMPL_SENDER(  ComplementaryInterface::QueryingQueries(int param),
                    AnsweringQueries(param)                    
                 )

   IF_IMPL_QUERY(  int ComplementaryInterface::SendingMessages(int param),
                   ReceivingMessages(param),
                   (int)"default return value"
                )

/////////////////////////////////////////////////////////////////////////////

   How do I use it ?   - Disconnect/Connect notifications


   Usually the virtual methods notifyDisconnect(ed) or notifyConnect(ed)
   will be called within connect/disconnect methods.

   As constructors and destructors are not able to call virtual methods
   of derived classes, there are two possible problems:

   * Constructors: Calling a connect method in a constructor will not result
     in a connect notification of any derived class. Thus do not use connect
     calls in contructors if any derived class hast to receive all
     connect/disconnect notifications.

   * Destructors: If connections are still present if the interface destructor
     is called, it will only call its own empty noticedisconnect method. That
     shouldn't be a big problem as the derived class is already gone and
     doesn't have any interest in this notification any more. But it might be
     possible that the connected object wants to call a function of the just
     destroyed derived class. That is not possible. Dynamic casts to the
     derived class will return NULL. Do not try to call methods of this class
     by use of cached pointers.
   


/////////////////////////////////////////////////////////////////////////////

   Extending and Aggregating Interfaces

   Our interfaces must be extended by aggregation. The reason is that
   otherwise we would have the same problems as with a common base class
   for connection management. Each interface extensions is an normal
   interface on its own.

   Example:

   class I_AM_FM_Radio : public IRadioBase,
                         public IRadioFrequencyExtension,
                         public IRadioSeekExtension
   {
   	...
   };

   To guarantee, that connection management continues to work, we have to overwrite
   the connect and disconnect methods:

     virtual bool I_AM_FM_Radio::connect (Interface *i) {
     	IRadioBase::connect(i);
     	IFrequencyExtension::connect(i);
     	ISeekExtension::connect(i);
     }

     virtual bool I_AM_FM_Radio::disconnect (Interface *i) {
     	IRadioBase::disconnect(i);
     	IFrequencyExtension::disconnect(i);
     	ISeekExtension::disconnect(i);
     }

*/


/////////////////////////////////////////////////////////////////////////////

// a polymorphic and *virtual* base class so that we can make use of
// dynamic_casts in connect/disconnect and to be able to merge
// connect/disconnect methods to one single function in case of multiple
// inheritance

class Interface
{
public:
	Interface () {}
	virtual ~Interface() {}

	virtual bool     connect   (Interface *) { return false; }
	virtual bool     disconnect(Interface *) { return false; }

	// "Interface &"-Versions for convienience, not virtual, only "Interface*"
	// versions have to / may  be overwritten in case of multiple inheritance
	bool     connect   (Interface &i) { return connect    (&i); }
	bool     disconnect(Interface &i) { return disconnect (&i); }
};

/////////////////////////////////////////////////////////////////////////////

template <class thisIF, class cmplIF>
class InterfaceBase : virtual public Interface
{
private:
	typedef InterfaceBase<thisIF, cmplIF>  thisClass;
	typedef InterfaceBase<cmplIF, thisIF>  cmplClass;

    friend class cmplClass; // necessary for connects (to keep number of different connect functions low)

public:
                                               
	typedef thisIF                    thisInterface;
	typedef cmplIF                    cmplInterface;

	typedef QPtrList<cmplIF>          IFList;
	typedef QPtrListIterator<cmplIF>  IFIterator;

	typedef thisClass BaseClass;
    
public :
	InterfaceBase (int maxConnections = -1);
	virtual ~InterfaceBase ();

	// duplicate connects will add no more entries to connection list
	virtual bool     connect(Interface *i);
	virtual bool     disconnect(Interface *i);

protected:
	virtual void     disconnectAll();

	
public:

   	// It might be compfortable to derived Interfaces to get an argument
	// of the Interface class, but that part of the object might
	// already be destroyed. Thus it is necessary to evaluate the additional
	// pointer_valid argument. A null pointer is not transmitted, as the
	// pointer value might be needed to clean up some references in derived
	// classes
	virtual void     noticeConnect     (cmplInterface *, bool /*pointer_valid*/) {}
	virtual void     noticeConnected   (cmplInterface *, bool /*pointer_valid*/) {}
	virtual void     noticeDisconnect  (cmplInterface *, bool /*pointer_valid*/) {}
	virtual void     noticeDisconnected(cmplInterface *, bool /*pointer_valid*/) {}

	virtual bool     isConnectionFree() const;
	virtual unsigned connected()        const { return connections.count(); }

protected :

	IFList connections;	
	int    maxConnections;

private:
	thisInterface *me;
	bool           me_valid;
};


// macros for interface declaration

#define INTERFACE(IF, cmplIF) \
	class IF; \
	class cmplIF; \
	class IF : public InterfaceBase<IF, cmplIF> \

#define IF_CON_DESTRUCTOR(IF, n) \
	IF() : BaseClass((n)) {} \
	virtual ~IF() { }

// macros to make sending messages or queries easier


// debug util
#ifdef DEBUG
    #include <iostream>
    using namespace std;
    #define IF_QUERY_DEBUG \
        if (connections.count() > 1) { \
            kdDebug() << "class " << typeid(this).name() << ": using IF_QUERY with #connections > 1\n"; \
        }
#else
    #define IF_QUERY_DEBUG
#endif



// messages

#define SENDERS   protected
#define RECEIVERS public

#define IF_SENDER(decl) \
		virtual int decl const;

#define IF_SEND_MESSAGE(call) \
		int ____n = 0; \
		for (IFIterator i(connections); i.current(); ++i) {   \
			if (i.current()->call ) ++____n; \
		}  \
		return ____n;

#define IF_IMPL_SENDER(decl, call) \
		int decl const \
		{ \
			IF_SEND_MESSAGE(call) \
		}

#define IF_RECEIVER(decl) \
		virtual bool decl = 0;


// queries

#define ANSWERS public
#define QUERIES protected

#define IF_QUERY(decl) \
		virtual decl const;

#define IF_SEND_QUERY(call, default) \
		cmplInterface *o = IFIterator(connections).current(); \
		if (o) { \
			IF_QUERY_DEBUG \
			return o->call; \
		} else { \
			return default; \
		} \
		
#define IF_IMPL_QUERY(decl, call, default) \
		decl const { \
			IF_SEND_QUERY(call, default) \
		}

#define IF_ANSWER(decl) \
		virtual decl = 0;


/////////////////////////////////////////////////////////////////////////////


template <class thisIF, class cmplIF>
InterfaceBase<thisIF, cmplIF>::InterfaceBase(int _maxConnections)
  : maxConnections(_maxConnections),
    me(NULL),
    me_valid(false)
{
}


template <class thisIF, class cmplIF>
InterfaceBase<thisIF, cmplIF>::~InterfaceBase()
{
	kdDebug() << "InterfaceBase Destructor" << endl;

	me_valid = false;
	// In this state the derived interfaces may already be destroyed
	// so that dereferencing cached upcasted me-pointers in noticeDisconnect(ed)
	// will fail.
	// Thus we must ensure that disconnectAll() is called in the (upper) thisIF
	// destructor, not here (see macro IF_CON_DESTRUCTOR).
	// If this has not taken place (i.e. the programmer forgot to do so)
	// we can only warn, clear our list now and hope that nothing
	// more bad will happen

	if (connections.count() > 0) {
		disconnectAll();
	}
}


template <class thisIF, class cmplIF>
bool InterfaceBase<thisIF, cmplIF>::isConnectionFree () const
{
	int m = maxConnections;
	return  (m < 0) || (connections.count() < (unsigned) m);
}


template <class thisIF, class cmplIF>
bool InterfaceBase<thisIF, cmplIF>::connect (Interface *__i)
{
	// cache upcasted pointer, especially important for disconnects
	// where already destructed derived parts cannot be reached with dynamic casts
	if (!me) me = dynamic_cast<thisIF*>(this);
	me_valid = me != NULL;

	// same with the other interface
	cmplClass *_i = dynamic_cast<cmplClass*>(__i);
	if (!_i) return false;
		
	if (!_i->me) _i->me = dynamic_cast<cmplIF*>(_i);		
	cmplIF    *i = _i->me;
	_i->me_valid = _i->me != NULL;
	
	if (i && me) {
		bool i_connected  = connections.containsRef(i);
		bool me_connected = i->connections.containsRef(me);

		if (i_connected || me_connected) {
			return true;
		} else if (isConnectionFree() && i->isConnectionFree()) {

			noticeConnect(i, i != NULL);
			_i->noticeConnect(me, me != NULL);
		
			if (!i_connected)
				connections.append(i);
			if (!me_connected)
				_i->connections.append(me);

			noticeConnected(i, i != NULL);
			_i->noticeConnected(me, me != NULL);
			
			return true;
		} else {
			return false;
		}
	}
	return false;
}



template <class thisIF, class cmplIF>
bool InterfaceBase<thisIF, cmplIF>::disconnect (Interface *__i)
{
	kdDebug() << "InterfaceBase::disconnect(" << __i << ") ENTER" << endl;
	cmplClass *_i = dynamic_cast<cmplClass*>(__i);

	kdDebug() << "cmplClass = " << _i << endl;

	// use cache to find pointer in connections list
	cmplIF *i = _i ? _i->me : NULL;

	kdDebug() << "cmplIF = " << i << endl;

	// The cached me pointer might already point to an destroyed
	// object. We must use it only for identifying the entry in
	// connections list

    if (i) {
		kdDebug() << "this->noticeDisconnect" << endl;
		if (me_valid)
			noticeDisconnect(i, _i->me_valid);
    }

	if (me) {		
		kdDebug() << "_i->noticeDisconnect" << endl;
		if (_i->me_valid)
			_i->noticeDisconnect(me, me_valid);
	}
		
	kdDebug() << "this->removeRef(i)" << endl;
	if (i && connections.containsRef(i))
		connections.removeRef(i);
		
	kdDebug() << "i->removeRef(me)" << endl;
	if (me && i && i->connections.containsRef(me))
		i->connections.removeRef(me);
		
	kdDebug() << "this->noticeDisconnected" << endl;
	if (me_valid && i)
		noticeDisconnected(i, _i->me_valid);
	kdDebug() << "_i->noticeDisconnected" << endl;
	if (_i->me_valid && me)
		_i->noticeDisconnected(me, me_valid);
		
	kdDebug() << "InterfaceBase::disconnect EXIT" << endl;
	return true;
}


template <class thisIF, class cmplIF>
void InterfaceBase<thisIF, cmplIF>::disconnectAll()
{
	while (connections.count()) {
		cmplIF *i = connections.getFirst();
		if (i) disconnect(i);
	}
}

#endif
