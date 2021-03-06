/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYBUNDLEDIRECTORY_H_
#define BERRYBUNDLEDIRECTORY_H_

#include <Poco/Path.h>

#include "berryIBundleStorage.h"


namespace berry {

using namespace Poco;

class BundleDirectory : public IBundleStorage
{

private:
  Path m_RootPath;

public:

  berryObjectMacro(BundleDirectory);

  BundleDirectory(const Path& path);
  virtual ~BundleDirectory();

  std::istream* GetResource(const std::string& path) const;
  void List(const std::string& path, std::vector<std::string>& files, bool quiet = true) const;
  bool IsDirectory(const std::string& path) const;
  Path GetPath() const;

  Path BuildPath(const std::string& path) const;

  bool operator==(const Object* o) const;
};

}  // namespace berry

#endif /*BERRYBUNDLEDIRECTORY_H_*/
