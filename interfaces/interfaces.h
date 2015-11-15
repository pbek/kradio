/***************************************************************************
                          interfaces.h  -  description
                             -------------------
    begin                : Fre Feb 28 2003
    copyright            : (C) 2003 by Martin Witte
    email                : emw-kradio@nocabal.de
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

#include <QtCore/QList>
#include <QtCore/QMap>
#include <kdebug.h>
#include <typeinfo>

#include <kdemacros.h>

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

class KDE_EXPORT Interface
{
public:
    Interface () {}
    virtual ~Interface() {}

    virtual bool     connectI   (Interface *) { return false; }
    virtual bool     disconnectI(Interface *) { return false; }

    // "Interface &"-Versions for convienience, not virtual, only "Interface*"
    // versions have to / may  be overwritten in case of multiple inheritance
    bool     connectI   (Interface &i) { return connectI    (&i); }
    bool     disconnectI(Interface &i) { return disconnectI (&i); }
};

/////////////////////////////////////////////////////////////////////////////

template <class thisIF, class cmplIF>
class KDE_EXPORT InterfaceBase : virtual public Interface
{
private:
    typedef InterfaceBase<thisIF, cmplIF>  thisClass;
    typedef InterfaceBase<cmplIF, thisIF>  cmplClass;

public:
    typedef cmplIF cmplInterface;
    typedef thisIF thisInterface;

//    friend class cmplClass; // necessary for connects (to keep number of different connect functions low)

public:

    typedef          QList<cmplInterface*>                  IFList;
    typedef typename QList<cmplInterface*>::iterator        IFIterator;
    typedef typename QList<cmplInterface*>::const_iterator  IFConstIterator;
    typedef          QList<IFList*>                         IFFineList;
    typedef typename QList<IFList*>::iterator               IFFineIterator;
    typedef typename QList<IFList*>::const_iterator         IFFineConstIterator;

    typedef thisClass BaseClass;

public :
    InterfaceBase (int maxIConnections = -1);
    virtual ~InterfaceBase ();

    // duplicate connects will add no more entries to connection list
    virtual bool     connectI   (Interface *i);
    virtual bool     disconnectI(Interface *i);

protected:
    virtual void     disconnectAllI();


public:

    // It might be compfortable to derived Interfaces to get an argument
    // of the Interface class, but that part of the object might
    // already be destroyed. Thus it is necessary to evaluate the additional
    // pointer_valid argument. A null pointer is not transmitted, as the
    // pointer value might be needed to clean up some references in derived
    // classes
    virtual void     noticeConnectI     (cmplInterface *, bool /*pointer_valid*/) {}
    virtual void     noticeConnectedI   (cmplInterface *, bool /*pointer_valid*/) {}
    virtual void     noticeDisconnectI  (cmplInterface *, bool /*pointer_valid*/);
    virtual void     noticeDisconnectedI(cmplInterface *, bool /*pointer_valid*/) {}

    virtual bool     isIConnectionFree() const;
    virtual unsigned connectedI()        const { return iConnections.count(); }

    thisInterface *initThisInterfacePointer();
    thisInterface *getThisInterfacePointer()                const { return me; }
    bool           isThisInterfacePointerValid()            const { return me_valid; }
    bool           hasConnectionTo   (cmplInterface *other) const { return iConnections.contains(other); }
    void           appendConnectionTo(cmplInterface *other)       { iConnections.append(other); }
    void           removeConnectionTo(cmplInterface *other)       { iConnections.removeAll(other); }

protected :

    IFList iConnections;
    int    maxIConnections;

 // functions for individually selectable callbacks
protected:
    bool addListener   (const cmplInterface *i, IFList &list);
    void removeListener(const cmplInterface *i, IFList &list);
    void removeListener(const cmplInterface *i);

    QMap<cmplInterface *, IFFineList >  m_FineListeners;

private:
    thisInterface *me;
    bool           me_valid;
};


// macros for interface declaration

#define INTERFACE(xIF, xcmplIF) \
    class xIF; \
    class xcmplIF; \
    class KDE_EXPORT xIF : public InterfaceBase<xIF, xcmplIF> \

#define IF_CON_DESTRUCTOR(IF, n) \
    IF() : BaseClass((n)) {} \
    virtual ~IF() { }

// macros to make sending messages or queries easier


// debug util
#ifdef DEBUG
    #include <iostream>
    using namespace std;
    #define IF_QUERY_DEBUG \
        if (iConnections.count() > 1) { \
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
        for (IFConstIterator i = iConnections.begin(); i != iConnections.end(); ++i) {   \
            if ((*i)->call ) ++____n; \
        }  \
        return ____n;

#define IF_IMPL_SENDER(decl, call) \
        int decl const \
        { \
            IF_SEND_MESSAGE(call) \
        }

#define IF_RECEIVER(decl) \
        virtual bool decl = 0;

#define IF_RECEIVER_EMPTY(decl) \
        virtual bool decl { return false; }

// queries

#define ANSWERS public
#define QUERIES protected

#define IF_QUERY(decl) \
        virtual decl const;

#define IF_SEND_QUERY(call, default) \
        IFConstIterator it = iConnections.begin(); \
        if (it != iConnections.end() && *it) { \
            cmplInterface *o = *it; \
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
// MACROS for individually selectable callbacks
/////////////////////////////////////////////////////////////////////////////


#define IF_SENDER_FINE(name, param) \
protected: \
    int  name param const; \
public: \
    bool register4_##name  (cmplInterface *); \
    void unregister4_##name(cmplInterface *); \
private: \
    IFList m_Listeners_##name;\


#define IF_SEND_MESSAGE_FINE(name, params, call) \
        int ____n = 0; \
        for (IFConstIterator ____it = m_Listeners_##name.begin(); ____it != m_Listeners_##name.end(); ++____it) {   \
            if ((*____it)->call ) ++____n; \
        }  \
        return ____n;

#define IF_IMPL_SENDER_FINE(class, name, param, call) \
    int class::name param const { \
        IF_SEND_MESSAGE_FINE(name, param, call) \
    } \
    \
    bool class::register4_##name(cmplInterface *i) {   \
        return addListener(i, m_Listeners_##name); \
    } \
    void class::unregister4_##name(cmplInterface *i) {   \
        m_Listeners_##name.removeAll(i);             \
    }


#define INLINE_IMPL_DEF_noticeConnectedI(BaseClass) \
    virtual void noticeConnectedI    (BaseClass::cmplInterface *i, bool pointer_valid) { BaseClass::noticeConnectedI    (i, pointer_valid); }

#define INLINE_IMPL_DEF_noticeConnectI(BaseClass) \
    virtual void noticeConnectI      (BaseClass::cmplInterface *i, bool pointer_valid) { BaseClass::noticeConnectI      (i, pointer_valid); }

#define INLINE_IMPL_DEF_noticeDisconnectedI(BaseClass) \
    virtual void noticeDisconnectedI (BaseClass::cmplInterface *i, bool pointer_valid) { BaseClass::noticeDisconnectedI (i, pointer_valid); }

#define INLINE_IMPL_DEF_noticeDisconnectI(BaseClass) \
    virtual void noticeDisconnectI   (BaseClass::cmplInterface *i, bool pointer_valid) { BaseClass::noticeDisconnectI   (i, pointer_valid); }


/////////////////////////////////////////////////////////////////////////////


template <class thisIF, class cmplIF>
InterfaceBase<thisIF, cmplIF>::InterfaceBase(int _maxIConnections)
  : maxIConnections(_maxIConnections),
    me(NULL),
    me_valid(false)
{
}


template <class thisIF, class cmplIF>
InterfaceBase<thisIF, cmplIF>::~InterfaceBase()
{
    me_valid = false;
    // In this state the derived interfaces may already be destroyed
    // so that dereferencing cached upcasted me-pointers in noticeDisconnect(ed)
    // will fail.
    // Thus we must ensure that disconnectAll() is called in the (upper) thisIF
    // destructor, not here (see macro IF_CON_DESTRUCTOR).
    // If this has not taken place (i.e. the programmer forgot to do so)
    // we can only warn, clear our list now and hope that nothing
    // more bad will happen

    if (iConnections.count() > 0) {
        thisClass::disconnectAllI();
    }
}


template <class thisIF, class cmplIF>
bool InterfaceBase<thisIF, cmplIF>::isIConnectionFree () const
{
    int m = maxIConnections;
    return  (m < 0) || (iConnections.count() < m);
}

template <class thisIF, class cmplIF>
thisIF *InterfaceBase<thisIF, cmplIF>::initThisInterfacePointer()
{
    if (!me) me = dynamic_cast<thisInterface *>(this);
    me_valid = me != NULL;
    return me;
}

template <class thisIF, class cmplIF>
bool InterfaceBase<thisIF, cmplIF>::connectI (Interface *__i)
{
    // cache upcasted pointer, especially important for disconnects
    // where already destructed derived parts cannot be reached with dynamic casts
    initThisInterfacePointer();

    // same with the other interface
    // requires -fvisibility=default for DSOs, otherwise rtti symbols
    // will be local in libs and exe and not compared correctly.
    // http://gcc.gnu.org/faq.html#dso
    cmplClass *_i = dynamic_cast<cmplClass *>(__i);
    if (!_i) {
        return false;
    }

    cmplInterface *i = _i->initThisInterfacePointer();

    if (i && me) {
        bool i_connected  = iConnections.contains(i);
        bool me_connected = i->hasConnectionTo(me);

        if (i_connected || me_connected) {
            return true;
        } else if (isIConnectionFree() && i->isIConnectionFree()) {

            noticeConnectI(i, i != NULL);
            _i->noticeConnectI(me, me != NULL);

            if (!i_connected)
                appendConnectionTo(i);
            if (!me_connected)
                _i->appendConnectionTo(me);

            noticeConnectedI(i, i != NULL);
            _i->noticeConnectedI(me, me != NULL);

            return true;
        } else {
            return false;
        }
    }
    return false;
}



template <class thisIF, class cmplIF>
bool InterfaceBase<thisIF, cmplIF>::disconnectI (Interface *__i)
{
    cmplClass *_i = dynamic_cast<cmplInterface *>(__i);

    // use cache to find pointer in connections list
    cmplInterface *i = _i ? _i->getThisInterfacePointer() : NULL;

    // The cached me pointer might already point to an destroyed
    // object. We must use it only for identifying the entry in
    // connections list

    if (i && _i) {
        if (me_valid)
            noticeDisconnectI(i, _i->isThisInterfacePointerValid());
    }

    if (me && _i) {
        if (_i->isThisInterfacePointerValid())
            _i->noticeDisconnectI(me, me_valid);
    }

    if (i && hasConnectionTo(i)) {
        removeListener(i);
        removeConnectionTo(i);
    }

    if (me && i && i->hasConnectionTo(me))
        i->removeConnectionTo(me);

    if (me_valid && i && _i)
        noticeDisconnectedI(i, _i->isThisInterfacePointerValid());
    if (_i && _i->isThisInterfacePointerValid() && me)
        _i->noticeDisconnectedI(me, me_valid);

    return true;
}


template <class thisIF, class cmplIF>
void InterfaceBase<thisIF, cmplIF>::noticeDisconnectI(cmplInterface *i, bool /*pointer_valid*/)
{
    removeListener(i);
}


template <class thisIF, class cmplIF>
void InterfaceBase<thisIF, cmplIF>::disconnectAllI()
{
    IFList tmp = iConnections;
    for (IFIterator it = tmp.begin(); it != tmp.end(); ++it) {
        /* Do not call virtual methods if I'm in the contstructor!
           Actually this should be ensured by the compiler generated
           code and virtual method tables, but unfortunately some compilers
           seem to ignore this in some situations.
         */
        if (me_valid)
            disconnectI(*it);
        else
            thisClass::disconnectI(*it);
    }
}




template <class thisIF, class cmplIF>
bool InterfaceBase<thisIF, cmplIF>::addListener(const cmplInterface *_i, IFList &list)
{
    // problem: const cmplInterface *i                    depicts a const instance and a mutable pointer
    //          list<cmplInterface*>::contains(const T&)  takes a constant pointer as parameter
    cmplInterface *i = const_cast<cmplInterface *>(_i);
    if (iConnections.contains(i) && !list.contains(i)) {
        list.append(i);
        m_FineListeners[i].append(&list);
        return true;
    } else {
        return false;
    }
}


template <class thisIF, class cmplIF>
void InterfaceBase<thisIF, cmplIF>::removeListener(const cmplInterface *i, IFList &list)
{
    list.removeAll(i);
    if (m_FineListeners.contains(i))
        m_FineListeners[i].remove(&list);
}


template <class thisIF, class cmplIF>
void InterfaceBase<thisIF, cmplIF>::removeListener(const cmplInterface *_i)
{
    cmplInterface *i = const_cast<cmplInterface*>(_i);
    if (m_FineListeners.contains(i)) {
        IFFineList        &list = m_FineListeners[i];
        IFFineIterator     it   = list.begin();
        for (; it != list.end(); ++it) {
            (*it)->removeAll(i);
        }
    }
    m_FineListeners.remove(i);
}







#endif
