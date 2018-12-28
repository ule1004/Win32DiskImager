
// Win32DiskImagerDlg.h: 헤더 파일
//

#pragma once
#include "CDisk.h"

#define MAX_DEVICE_CNT		40
#define USER_TIMER			1
#define TIMER_TIME			100

typedef struct _DEVICE_INFO
{
	char deviceName[8];
	int deviceID;
	int volumeID;
} DEVICE_INFO, *PDEVICE_INFO;

enum Status {
	STATUS_IDLE = 0,
	STATUS_READING,
	STATUS_WRITING,
	STATUS_EXIT,
	STATUS_CANCELED
};


// CWin32DiskImagerDlg 대화 상자
class CWin32DiskImagerDlg : public CDialogEx
{
// 생성입니다.
public:
	CWin32DiskImagerDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WIN32DISKIMAGER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_comboDevice;
	CProgressCtrl m_ProgressBar;
	CMFCEditBrowseCtrl m_editPwd;
	int status;
	CWinThread *pthread;
	void getLogicalDrives();
	DEVICE_INFO deviceInfo[MAX_DEVICE_CNT];
	void SetDisableDlg();
	afx_msg void OnBnClickedBtnRead();
	afx_msg void OnBnClickedBtnWrite();
	afx_msg void OnBnClickedBtnCancel();
	void OnRead();
	void OnWrite();
	HANDLE hVolume;
	HANDLE hFile;
	HANDLE hRawDisk;
	unsigned long long sectorsize;
	char *sectorData;
	BOOL OnDeviceChange(UINT nEventType, DWORD dwdata);
	afx_msg void OnChangeMfceditPwd();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	unsigned long long ull_maxTime;
	unsigned long long ull_userTime;
	int timer_cnt;
};
