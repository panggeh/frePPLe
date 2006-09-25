/***************************************************************************
  file : $URL: file:///develop/SVNrepository/frepple/trunk/src/dllmain.cpp $
  version : $LastChangedRevision$  $LastChangedBy$
  date : $LastChangedDate$
  email : jdetaeye@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 * Copyright (C) 2006 by Johan De Taeye                                    *
 *                                                                         *
 * This library is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU Lesser General Public License as published   *
 * by the Free Software Foundation; either version 2.1 of the License, or  *
 * (at your option) any later version.                                     *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser *
 * General Public License for more details.                                *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA *
 *                                                                         *
 ***************************************************************************/

#define FREPPLE_CORE 
#include "frepple.h"
#include "freppleinterface.h"
using namespace frepple;
#include <sys/stat.h>


#if defined(WIN32) && !defined(STATIC)
// This function is only applicable for the windows operating systems
// and when it hasn't been explicitly disabled by setting the STATIC variable.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

BOOL APIENTRY DllMain(HANDLE hInst, DWORD ul_reason_for_call, LPVOID lpReserved)
{
  switch (ul_reason_for_call)
  {
    case DLL_PROCESS_ATTACH:
      // Loading the library
      try { FreppleInitialize(NULL); }
      catch (exception& e) 
      {
        cout << "Error: " << e.what() << endl;
        return FALSE;
      }
      catch (...)
      {
        cout << "Error: Unknown exception type" << endl;
        return FALSE;
      }
      return TRUE;

    case DLL_PROCESS_DETACH:
      // Unloading the library
      FreppleWrapperExit();
      return TRUE;
  }
  return TRUE;
}
#endif


DECLARE_EXPORT(void) FreppleInitialize(const char* h)
{
  static bool initialized = false;
  if (!initialized)
  {
    // If a parameter is given we set the environment variable FREPPLE_HOME.
    // If the parameter is NULL, we pick up the existing value of that 
    // variable.
    if (h) Environment::setHomeDirectory(h);
    else
    {
      const char *c = getenv("FREPPLE_HOME");
      if (c) Environment::setHomeDirectory(c);
      else clog << "Warning: No valid home directory specified" << endl;
    }

    // Initialize the libraries
    LibraryModel::initialize(); // also initializes the utils library
    LibrarySolver::initialize();  
      
    // Search for the initialization file
    if (!Environment::getHomeDirectory().empty())
    {
      string init(Environment::getHomeDirectory());
      init += "init.xml";
      struct stat stat_p;
      if (!stat(init.c_str(), &stat_p))
      {
        // File exists
        if (!(stat_p.st_mode & S_IREAD))
          // File exists but is not readable
          clog << "Warning: Initialization file 'init.xml'"
            << " exists but is not readable" << endl;
        else
          // Execute the commands in the file
          try{ CommandReadXMLFile(init).execute(); }
          catch (...)
          {
            clog << "Exception caught during execution of 'init.xml'" << endl;
            throw;
          }
      }
    }

    // Avoid executing this multiple times
    initialized = true;
  }
}


DECLARE_EXPORT(void) FreppleReadXMLData (char* x, bool validate, bool validateonly)
{
  if (x) CommandReadXMLString(string(x), validate, validateonly).execute();
}


DECLARE_EXPORT(void) FreppleReadXMLFile (const char* x, bool validate, bool validateonly)
{
  CommandReadXMLFile(x, validate, validateonly).execute();
}


DECLARE_EXPORT(void) FreppleSaveFile(char* x)
{
  CommandSave(x).execute();
}


DECLARE_EXPORT(string) FreppleSaveString()
{
  XMLOutputString x;
  x.writeElementWithHeader(Tags::tag_plan, &Plan::instance());
  return x;
}


DECLARE_EXPORT(void) FreppleExit()
{
  // Shut down the fepple executable, or the application that loaded frepple
  // as a dynamic library
  Plan::instance().setLogFile(""); // Close the log file
  std::exit(EXIT_SUCCESS);
}

 
extern "C" DECLARE_EXPORT(int) FreppleWrapperInitialize(const char* h) 
{
  try {FreppleInitialize(h);}
  catch (...) {return EXIT_FAILURE;}
  return EXIT_SUCCESS;
}


extern "C" DECLARE_EXPORT(int) FreppleWrapperReadXMLData(char* d, bool v, bool c)
{
  try {FreppleReadXMLData(d, v, c);}
  catch (...) {return EXIT_FAILURE;}
  return EXIT_SUCCESS;
}


extern "C" DECLARE_EXPORT(int) FreppleWrapperReadXMLFile(const char* f, bool v, bool c) 
{
  try {FreppleReadXMLFile(f, v, c);}
  catch (...) {return EXIT_FAILURE;}
  return EXIT_SUCCESS;
}


extern "C" DECLARE_EXPORT(int) FreppleWrapperSaveFile(char* f) 
{
  try {FreppleSaveFile(f);}
  catch (...) {return EXIT_FAILURE;}
  return EXIT_SUCCESS;
}


extern "C" DECLARE_EXPORT(int) FreppleWrapperSaveString(char* buf, unsigned long sz) 
{
  try
  {
    // Get the result
    string result = FreppleSaveString();
    // Copy into the reply buffer
    unsigned long l = result.size();
    memcpy(buf, result.data(), l>sz ? sz : l);
  }
  catch (...) {return EXIT_FAILURE;}  
  return EXIT_SUCCESS;
}


extern "C" DECLARE_EXPORT(int) FreppleWrapperExit() 
{
  try {FreppleExit();}
  catch (...) {return EXIT_FAILURE;}
  return EXIT_SUCCESS;
}

