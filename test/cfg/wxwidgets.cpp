
// Test library configuration for windows.cfg
//
// Usage:
// $ cppcheck --check-library --library=windows --enable=information --error-exitcode=1 --inline-suppr --suppress=missingIncludeSystem test/cfg/windows.cpp
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <wx/filefn.h>

void validCode()
{
    wxString str = wxGetCwd();

    wxLogGeneric(wxLOG_Message, "test %d", 0);
    wxLogMessage("test %s", "str");

    wxSpinCtrl::SetBase(10);
    wxSpinCtrl::SetBase(16);

    wxString translation1 = _("text");
    wxString translation2 = wxGetTranslation("text");
    wxString translation3 = wxGetTranslation("string", "domain");
}

void nullPointer()
{
    // cppcheck-suppress nullPointer
    wxLogGeneric(wxLOG_Message, NULL);
    // cppcheck-suppress nullPointer
    wxLogMessage(NULL);
}

void ignoredReturnValue()
{
    // cppcheck-suppress ignoredReturnValue
    wxGetCwd();
}

void invalidFunctionArg()
{
    // cppcheck-suppress invalidFunctionArg
    wxSpinCtrl::SetBase(0);
    // cppcheck-suppress invalidFunctionArg
    wxSpinCtrl::SetBase(5);
}

void uninitvar()
{
    wxLogLevel logLevelUninit;
    char cBufUninit[10];
    char *pcUninit;
    // cppcheck-suppress uninitvar
    wxLogGeneric(logLevelUninit, "test");
    // cppcheck-suppress uninitvar
    wxLogMessage(cBufUninit);
    // cppcheck-suppress uninitvar
    wxLogMessage(pcUninit);
}
