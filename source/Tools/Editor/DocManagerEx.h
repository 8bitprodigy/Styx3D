// CDocManager class declaration
//

#include "stdafx.h"

class CDocManagerEx : public CDocManager
{
	DECLARE_DYNAMIC(CDocManagerEx)

// Construction
public:
	CDocManagerEx();

// Attributes
public:
	// Krouer: addition to keep current directory in search for drivers
	CString m_szCurrentPath;

// Operations
public:

// Overrides
	virtual BOOL DoPromptFileName(CString& fileName, UINT nIDSTitle,
			DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate);

// Implementation
    void RegisterOtherFileTypes();
public:
	virtual ~CDocManagerEx();
};


