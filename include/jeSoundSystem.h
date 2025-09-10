/*!
	@file jeSoundSystem.h 
	
	@author Anthony Rufrano
	@brief New and improved sound system

	@par Licence
	The contents of this file are subject to the Jet3D Public License       
	Version 1.02 (the "License"); you may not use this file except in         
	compliance with the License. You may obtain a copy of the License at       
	http://www.jet3d.com                                                        
                                                                             
	@par
	Software distributed under the License is distributed on an "AS IS"           
	basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See           
	the License for the specific language governing rights and limitations          
	under the License.                                                               
                                                                                  
	@par
	The Original Code is Jet3D, released December 12, 1999.                            
	Copyright (C) 1996-1999 Eclipse Entertainment, L.L.C. All Rights Reserved           
*/

/*!  @note  SoundSystem
*	This object represents the engine's sound core. This is the basic entity of the rendering engine.
*/
#ifndef JE_SOUNDSYSTEM_H
#define JE_SOUNDSYSTEM_H

#include "BaseType.h"
#include "VFile.h"

/*!
*	@typedef jeSoundSystem
*	@brief The sound system structure
*/
typedef struct jeSoundSystem					jeSoundSystem;

/*!
*	@typedef jeSound
*	@brief A reference to a sound buffer
*/
typedef struct jeSound							jeSound;

/*!
*	@typedef jeSound3d
*	@brief A 3D sound
*/
typedef struct jeSound3d						jeSound3d;

/*!
*	@typedef jeSoundListener
*	@brief Represents the position of the player in the world relative to all 3D sounds.
*/
typedef struct jeSoundListener					jeSoundListener;

/*!
*	@fn jeSoundSystem *jeSoundSystem_Create(HWND hWnd, uint32 Flags)
*	@brief Creates the sound system
*	@param[in] hWnd The window handle to attach the sound system to.
*   @param[in] Flags The sound system creation flags.
*	@return The created sound system.  NULL if unsuccessful.
*/
JETAPI jeSoundSystem *JETCC jeSoundSystem_Create(HWND hWnd, uint32 Flags);

/*!
*	@fn uint32 jeSoundSystem_CreateRef(jeSoundSystem *SoundSys)
*	@brief Increments the reference counter for the sound system.
*	@param[in] SoundSys The sound system to reference.
*	@return The number of current references
*/
JETAPI uint32 JETCC jeSoundSystem_CreateRef(jeSoundSystem *SoundSys);

/*!
*	@fn uint32 jeSoundSystem_Destroy(jeSoundSystem **SoundSys)
*	@brief Dereferences the sound system.  Destroys it if there are no more references.
*	@param[in] SoundSys The sound system to dereference.
*	@return The number of current references
*/
JETAPI uint32 JETCC jeSoundSystem_Destroy(jeSoundSystem **SoundSys);

/*!
*	@fn jeSound *jeSoundSystem_LoadSound(jeSoundSystem *SoundSys, jeVFile *File, uint32 Flags)
*	@brief Loads a sound from a file.
*	@param[in] SoundSys The active sound system.
*	@param[in] File The file to load it from.
*   @param[in] Flags The flags of the sound.
*	@return The loaded sound.  NULL on failure.
*/
JETAPI jeSound *JETCC jeSoundSystem_LoadSound(jeSoundSystem *SoundSys, jeVFile *File, uint32 Flags);

/*!
*	@fn jeBoolean jeSoundSystem_DestroySound(jeSoundSystem *SoundSys, jeSound **Snd)
*	@brief Destroys a loaded sound.
*	@param[in] SoundSys The active sound system
*	@param[in] Snd The sound to destroy.
*	@return JE_TRUE on success, JE_FALSE on failure
*	@note Failure will result in a memory leak!!!
*/
JETAPI jeBoolean JETCC jeSoundSystem_DestroySound(jeSoundSystem *SoundSys, jeSound **Snd);

/*!
*	@fn jeSound *jeSoundSystem_PlaySound(jeSoundSystem *SoundSys, jeSound *Snd)
*	@brief Plays a sound
*	@param[in] SoundSys The active sound system
*	@param[in] Snd The sound to play
*	@return JE_TRUE on success, JE_FALSE on failure
*/
JETAPI jeBoolean JETCC jeSoundSystem_PlaySound(jeSoundSystem *SoundSys, jeSound *Snd);

/*!
*	@fn jeBoolean jeSoundSystem_StopSound(jeSoundSystem *SoundSys, jeSound *Snd)
*	@brief Stops a sound
*	@param[in] SoundSys The active sound system
*	@param[in] Snd The sound to stop
*	@return JE_TRUE on success, JE_FALSE on failure
*/
JETAPI jeBoolean JETCC jeSoundSystem_StopSound(jeSoundSystem *SoundSys, jeSound *Snd);

/*!
*	@fn jeBoolean jeSoundSystem_PauseSound(jeSoundSystem *SoundSys, jeSound *Snd)
*	@brief Pauses a sound
*	@param[in] SoundSys The active sound system
*	@param[in] Snd The sound to pause
*	@return JE_TRUE on success, JE_FALSE on failure
*/
JETAPI jeBoolean JETCC jeSoundSystem_PauseSound(jeSoundSystem *SoundSys, jeSound *Snd);

/*!
*	@fn jeBoolean jeSoundSystem_IsPlaying(jeSoundSystem *SoundSys, jeSound *Snd)
*	@brief Checks if a sound is playing
*	@param[in] SoundSys The active sound system
*	@param[in] Snd The sound to query
*	@return JE_TRUE if the sound is playing, JE_FALSE if not
*/
JETAPI jeBoolean JETCC jeSoundSystem_IsPlaying(jeSoundSystem *SoundSys, jeSound *Snd);

/*!
*	@fn jeBoolean jeSoundSystem_IsPaused(jeSoundSystem *SoundSys, jeSound *Snd)
*	@brief Checks if a sound is paused
*	@param[in] SoundSys The active sound system
*	@param[in] Snd The sound to query
*	@return JE_TRUE if the sound is paused, JE_FALSE if not
*/
JETAPI jeBoolean JETCC jeSoundSystem_IsPaused(jeSoundSystem *SoundSys, jeSound *Snd);

/*!
*	@fn jeBoolean jeSoundSystem_IsLooping(jeSoundSystem *SoundSys, jeSound *Snd)
*	@brief Checks if a sound is looping
*	@param[in] SoundSys The active sound system
*	@param[in] Snd The sound to query
*	@return JE_TRUE if the sound is looping, JE_FALSE if not
*/
JETAPI jeBoolean JETCC jeSoundSystem_IsLooping(jeSoundSystem *SoundSys, jeSound *Snd);

/*!
*	@fn float jeSound_GetVolume(jeSound *Snd)
*	@brief Gets the volume of a sound
*	@param[in] Snd The sound to query
*	@return The volume of the sound
*	@note The value will be between 0.0f and 1.0f
*/
JETAPI float JETCC jeSound_GetVolume(jeSound *Snd);

/*!
*	@fn jeBoolean jeSound_SetVolume(jeSound *Snd, float vol)
*	@brief Sets the volume of a sound
*	@param[in] Snd The sound to modify
*	@param[in] vol The new volume
*	@return JE_TRUE on success, JE_FALSE on failure
*	@note The value should be between 0.0f and 1.0f.  It can be modified while the sound is playing.
*/
JETAPI jeBoolean JETCC jeSound_SetVolume(jeSound *Snd, float vol);

/*!
*	@fn float jeSound_GetPan(jeSound *Snd)
*	@brief Gets the panning property of the sound
*	@param[in] Snd The sound to query
*	@return The panning value
*	@note This value will be between -1.0f and 1.0f
*/
JETAPI float JETCC jeSound_GetPan(jeSound *Snd);

/*!
*	@fn jeBoolean jeSound_SetPan(jeSound *Snd, float pan)
*	@brief Sets the panning property of the sound
*	@param[in] Snd The sound to modify
*	@param[in] pan The new panning value
*	@return JE_TRUE on success, JE_FALSE on failure
*	@note The value should be between -1.0f and 1.0f.  It can be modified while the sound is playing.
*/
JETAPI jeBoolean JETCC jeSound_SetPan(jeSound *Snd, float pan);
