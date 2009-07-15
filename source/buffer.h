/****************************************************************************
 * USB Loader GX Team
 * buffer.cpp
 *
 * Loading covers in a background thread
 ***************************************************************************/

#ifndef _BUFFER_H_
#define _BUFFER_H_

GuiImage * ImageBuffer(int imagenumber);
void NewOffset(int off, int d);
void InitBufferThread();
void ShutdownBufferThread();
void ResumeBufferThread(int offset);
void HaltBufferThread();

#endif
