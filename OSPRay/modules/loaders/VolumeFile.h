// ======================================================================== //
// Copyright 2009-2015 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#pragma once

#include <stdlib.h>
#include <string>
#include "ospray/include/ospray/ospray.h"

//! \brief Define a function to create an instance of the InternalClass
//!  associated with ExternalName.
//!
//! \internal The function generated by this macro is used to create an
//!  instance of a concrete subtype of an abstract base class.  This
//!  macro is needed since the subclass type may not be known to OSPRay
//!  at build time.  Rather, the subclass can be defined in an external
//!  module and registered with OSPRay using this macro.
//! 
#define OSP_REGISTER_VOLUME_FILE(InternalClass, ExternalName)           \
  extern "C" OSPVolume ospray_import_volume_file_##ExternalName(const std::string &filename, OSPVolume volume) \
    { InternalClass file(filename);  return(file.importVolume(volume)); }

//! \brief The VolumeFile class is an abstraction for the concrete
//!  object which is used to load volume data from a file.
//!
//!  The format of the volume data stored in a file on disk is unknown
//!  to this class.  Subclasses implement loaders for specific formats,
//!  and the actual subclass used is determined from the file extension.
//!  This subclass must be registered in OSPRay proper, or in a loaded
//!  module via OSP_REGISTER_VOLUME_FILE.
//!
class VolumeFile {
public:

  //! Constructor.
  VolumeFile() {};

  //! Destructor.
  virtual ~VolumeFile() {};

  //! Create a VolumeFile object of the subtype given by the file extension and import the volume.
  static OSPVolume importVolume(const std::string &filename, OSPVolume volume);

  //! Import the volume specification and voxel data.
  virtual OSPVolume importVolume(OSPVolume volume) = 0;

  //! A string description of this class.
  virtual std::string toString() const { return("ospray_module_loaders::VolumeFile"); }

protected:

  //! Print an error message.
  void emitMessage(const std::string &kind, const std::string &message) const
    { std::cerr << "  " + toString() + "  " + kind + ": " + message + "." << std::endl; }

  //! Error checking.
  void exitOnCondition(bool condition, const std::string &message) const
    { if (!condition) return;  emitMessage("ERROR", message);  exit(1); }

  //! Warning condition.
  void warnOnCondition(bool condition, const std::string &message) const
    { if (!condition) return;  emitMessage("WARNING", message); }

  //! Get the absolute file path.
  static std::string getFullFilePath(const std::string &filename)
    { char *fullpath = realpath(filename.c_str(), NULL);  return(fullpath != NULL ? fullpath : filename); }

};
