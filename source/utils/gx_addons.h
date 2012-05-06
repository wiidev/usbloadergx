/****************************************************************************
 * Copyright (C) 2012 Dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#ifndef GX_ADDONS_H_
#define GX_ADDONS_H_

#include <gccore.h>

#ifdef __cplusplus
extern "C" {
#endif

void GX_Project(f32 mx, f32 my, f32 mz, Mtx mv, const f32 *projection,
				const f32 *viewport, f32 *sx, f32 *sy, f32 *sz);
void GX_GetProjectionv( f32* ptr, Mtx44 p, u8 type);
void GX_GetViewportv( f32* ptr, GXRModeObj *vmode );

#ifdef __cplusplus
}
#endif

#endif
