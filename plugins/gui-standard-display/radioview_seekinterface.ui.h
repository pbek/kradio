/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

void RadioView_SeekerUI::init()
{
	Accel = new QAccel (this);
	Accel->insertItem (Key_Left,  100);
	Accel->insertItem (Key_Right, 101);
	Accel->connectItem (100, sldRange, SLOT(subtractStep()));
	Accel->connectItem (101, sldRange, SLOT(addStep()));
}

void RadioView_SeekerUI::destroy()
{
}
