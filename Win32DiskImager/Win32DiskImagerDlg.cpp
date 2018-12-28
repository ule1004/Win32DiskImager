
// Win32DiskImagerDlg.cpp: 구현 파일
//

#include "stdafx.h"
#include "Win32DiskImager.h"
#include "Win32DiskImagerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/*****************************************************************************************************
 * Thread
 *****************************************************************************************************/

UINT pThreadFunc(LPVOID _dlg)
{
	CWin32DiskImagerDlg * Dlg = (CWin32DiskImagerDlg*)_dlg;
	switch (Dlg->status)
	{
	case STATUS_READING:
		Dlg->OnRead();
		break;
	case STATUS_WRITING:
		Dlg->OnWrite();
		break;
	default:
		break;
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
public:
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CWin32DiskImagerDlg 대화 상자



CWin32DiskImagerDlg::CWin32DiskImagerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_WIN32DISKIMAGER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWin32DiskImagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_DEVICE, m_comboDevice);
	DDX_Control(pDX, IDC_PROGRESS, m_ProgressBar);
	DDX_Control(pDX, IDC_MFCEDIT_PWD, m_editPwd);
}

BEGIN_MESSAGE_MAP(CWin32DiskImagerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DEVICECHANGE()
	ON_BN_CLICKED(IDC_BTN_READ, &CWin32DiskImagerDlg::OnBnClickedBtnRead)
	ON_BN_CLICKED(IDC_BTN_WRITE, &CWin32DiskImagerDlg::OnBnClickedBtnWrite)
	ON_BN_CLICKED(IDC_BTN_CANCEL, &CWin32DiskImagerDlg::OnBnClickedBtnCancel)
	ON_EN_CHANGE(IDC_MFCEDIT_PWD, &CWin32DiskImagerDlg::OnChangeMfceditPwd)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CWin32DiskImagerDlg 메시지 처리기

BOOL CWin32DiskImagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	GetDlgItem(IDC_COMBO_DEVICE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_READ)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_WRITE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_CANCEL)->EnableWindow(FALSE);

	status = STATUS_IDLE;
	pthread = NULL;
	getLogicalDrives();

	LPCTSTR szFilter = _T("Disk Images (*.img *.IMG)|*.img;*.IMG|All Files (*.*)|*.*||");
	m_editPwd.EnableFileBrowseButton(NULL, szFilter, OFN_HIDEREADONLY);
//	SetTimer(USER_TIMER, 1000, NULL);

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CWin32DiskImagerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CWin32DiskImagerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CWin32DiskImagerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CWin32DiskImagerDlg::getLogicalDrives()
{
	unsigned long driveMask = GetLogicalDrives();
	int i = 0;
	ULONG pID;
	int devicecnt = 0;
	CDisk cdisk;

	UpdateData(TRUE);
	m_comboDevice.ResetContent();

	while (driveMask != 0)
	{
		if (driveMask & 1)
		{
			char drivename[] = "\\\\.\\A:\\";
			drivename[4] += i;
			pID = 0;
			if (cdisk.checkDriveType(drivename, &pID))
			{
				CString comboWord;
				comboWord.Format("[%c:\\]", drivename[4]);
				m_comboDevice.AddString(comboWord);

				strcpy(deviceInfo[devicecnt].deviceName, drivename);
				deviceInfo[devicecnt].volumeID = i;
				deviceInfo[devicecnt].deviceID = pID;

				devicecnt++;
			}
		}
		driveMask >>= 1;
		++i;
	}

	if (devicecnt > 0)
	{
		GetDlgItem(IDC_COMBO_DEVICE)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_COMBO_DEVICE)->EnableWindow(FALSE);
	}

	m_comboDevice.SetCurSel(0);
	UpdateData(FALSE);

	SetDisableDlg();
}


void CWin32DiskImagerDlg::SetDisableDlg()
{
	CString path;
	CString device;
	GetDlgItemText(IDC_MFCEDIT_PWD, path);
	GetDlgItemText(IDC_COMBO_DEVICE, device);

	if (!path.IsEmpty() && !device.IsEmpty())
	{
		GetDlgItem(IDC_BTN_READ)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_WRITE)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_CANCEL)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_BTN_READ)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_WRITE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_CANCEL)->EnableWindow(FALSE);
	}
}


void CWin32DiskImagerDlg::OnBnClickedBtnRead()
{
	status = STATUS_READING;
	GetDlgItem(IDC_BTN_READ)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_WRITE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_CANCEL)->EnableWindow(TRUE);
	UpdateData(TRUE);
	pthread = AfxBeginThread(pThreadFunc, this);
}


void CWin32DiskImagerDlg::OnBnClickedBtnWrite()
{
	status = STATUS_WRITING;
	GetDlgItem(IDC_BTN_READ)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_WRITE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_CANCEL)->EnableWindow(TRUE);
	UpdateData(TRUE);
	pthread = AfxBeginThread(pThreadFunc, this);
}


void CWin32DiskImagerDlg::OnBnClickedBtnCancel()
{
	status = STATUS_CANCELED;
	SetDisableDlg();
	m_ProgressBar.SetRange(1000, 1000);
}


void CWin32DiskImagerDlg::OnRead()
{
	CDisk cdisk;
	CString path;
	GetDlgItemText(IDC_MFCEDIT_PWD, path);
	char diskName[4] = "\\\\";
	strncpy(diskName, path, 2);

	if (!path.IsEmpty())
	{
		int select = m_comboDevice.GetCurSel();

		int volumeID = deviceInfo[select].volumeID;
		int deviceID = deviceInfo[select].deviceID;

		unsigned long long numsectors;
		unsigned long long filesize;
		unsigned long long spaceneeded = 0ull;
		unsigned long long lasti;
		unsigned long long i;


		hVolume = cdisk.getHandleOnVolume(volumeID, GENERIC_READ);
		if (hVolume == INVALID_HANDLE_VALUE)
		{
			TRACE("%s INVALID_HANDLE_VALUE\n", __func__);
			status = STATUS_IDLE;
			SetDisableDlg();
			return;
		}

		if (!cdisk.getLockOnVolume(hVolume))
		{
			TRACE("%s getLockOnVolume\n", __func__);
			CloseHandle(hVolume);
			status = STATUS_IDLE;
			SetDisableDlg();
			return;
		}

		if (!cdisk.unmountVolume(hVolume))
		{
			TRACE("%s unmountVolume\n", __func__);
			cdisk.removeLockOnVolume(hVolume);
			CloseHandle(hVolume);
			status = STATUS_IDLE;
			SetDisableDlg();
			return;
		}

		hFile = cdisk.getHandleOnFile((LPSTR)(LPCTSTR(path)), GENERIC_WRITE);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			TRACE("%s getHandleOnFile\n", __func__);
			cdisk.removeLockOnVolume(hVolume);
			CloseHandle(hVolume);
			status = STATUS_IDLE;
			SetDisableDlg();
			return;
		}

		hRawDisk = cdisk.getHandleOnDevice(deviceID, GENERIC_READ);
		if (hRawDisk == INVALID_HANDLE_VALUE)
		{
			TRACE("%s getHandleOnDevice\n", __func__);
			cdisk.removeLockOnVolume(hVolume);
			CloseHandle(hVolume);
			CloseHandle(hFile);
			status = STATUS_IDLE;
			SetDisableDlg();
			return;
		}

		numsectors = cdisk.getNumberOfSectors(hRawDisk, &sectorsize);
		filesize = cdisk.getFileSizeInSectors(hFile, sectorsize);
		if (filesize >= numsectors)
		{
			spaceneeded = 0ull;
		}
		else
		{
			spaceneeded = (unsigned long long)(numsectors - filesize) * (unsigned long long)(sectorsize);
		}

		if (!cdisk.spaceAvailable(diskName, spaceneeded))
		{
			MessageBox(_T("Disk is not large enough for the specified image."), _T("Write Error"), MB_ICONEXCLAMATION | MB_ICONWARNING);
			cdisk.removeLockOnVolume(hVolume);
			CloseHandle(hVolume);
			CloseHandle(hFile);
			CloseHandle(hRawDisk);
			sectorData = NULL;
			status = STATUS_IDLE;
			SetDisableDlg();
			return;
		}

		if (numsectors == 0ul)
		{
			m_ProgressBar.SetRange(0, 100);
		}
		else
		{
			m_ProgressBar.SetRange(0, 1000);
		}

		lasti = 0ul;
		//timer.start();
		timer_cnt = 0;
		ull_maxTime = numsectors;
		SetTimer(USER_TIMER, TIMER_TIME, NULL);
		for (i = 0ul; i < numsectors && status == STATUS_READING; i += 1024ul)
		{
			sectorData = cdisk.readSectorDataFromHandle(hRawDisk, i, (numsectors - i >= 1024ul) ? 1024ul : (numsectors - i), sectorsize);
			if (sectorData == NULL)
			{
				TRACE("%s readSectorDataFromHandle\n", __func__);
				delete sectorData;
				sectorData = NULL;
				cdisk.removeLockOnVolume(hVolume);
				CloseHandle(hVolume);
				CloseHandle(hFile);
				CloseHandle(hRawDisk);
				status = STATUS_IDLE;
				KillTimer(USER_TIMER);
				SetDisableDlg();
				return;
			}
			if (!cdisk.writeSectorDataToHandle(hFile, sectorData, i, (numsectors - i >= 1024ul) ? 1024ul : (numsectors - i), sectorsize))
			{
				TRACE("%s writeSectorDataToHandle\n", __func__);
				delete sectorData;
				sectorData = NULL;
				cdisk.removeLockOnVolume(hVolume);
				CloseHandle(hVolume);
				CloseHandle(hFile);
				CloseHandle(hRawDisk);
				status = STATUS_IDLE;
				KillTimer(USER_TIMER);
				SetDisableDlg();
				return;
			}
			delete sectorData;
			sectorData = NULL;
			/*
			if (timer.elapsed() >= 1000)
			{
				mbpersec = (((double)sectorsize * (i - lasti)) * (1000.0 / timer.elapsed())) / 1024.0 / 1024.0;
				statusbar->showMessage(QString("%1MB/s").arg(mbpersec));
				timer.start();
				lasti = i;
			}
			progressbar->setValue(i);
			QCoreApplication::processEvents();
			*/
			m_ProgressBar.SetPos((int)(i * 1000 / numsectors));
			ull_userTime = i;
		}
		cdisk.removeLockOnVolume(hVolume);
		CloseHandle(hVolume);
		CloseHandle(hFile);
		CloseHandle(hRawDisk);
		KillTimer(USER_TIMER);
		sectorData = NULL;
		//m_ProgressBar.reset
		//statusbar->showMessage(tr("Done."));
		//bCancel->setEnabled(false);
		//setReadWriteButtonState();
		if (status == STATUS_CANCELED)
		{
			MessageBox(_T("Read Canceled."), _T("Complete"), MB_ICONEXCLAMATION | MB_ICONWARNING);
			//MessageBox::information(NULL, tr("Complete"), tr("Read Canceled."));
		}
		else
		{
			MessageBox(_T("Read Successful."), _T("Complete"), MB_ICONEXCLAMATION | MB_ICONWARNING);
			//QMessageBox::information(NULL, tr("Complete"), tr("Read Successful."));
		}
		/*
		if (md5CheckBox->isChecked())
		{
			QFileInfo fileinfo(myFile);
			if (fileinfo.exists() && fileinfo.isFile() &&
				fileinfo.isReadable() && (fileinfo.size() > 0))
			{
				generateMd5(myFile.toLatin1().data());
			}
		}
		*/
	}
	else
	{
		MessageBox(_T("Please specify a file to save data to."), _T("File Info"), MB_ICONEXCLAMATION | MB_ICONWARNING);
		//QMessageBox::critical(NULL, tr("File Info"), tr("Please specify a file to save data to."));
	}
	if (status == STATUS_EXIT)
	{
		//close();
	}
	status = STATUS_IDLE;
	SetDisableDlg();
}


void CWin32DiskImagerDlg::OnWrite()
{
	CDisk cdisk;
	bool passfail = true;

	/*
	if (!isURLCorrect())
	{
		if (!cbEnterprise->isChecked())
		{
			QMessageBox::critical(NULL, tr("URL Error"), tr("The URL is incorrect; the first part should consists of a maximum of 4 numbers, the second part should consist of numbers."));
		}
		return;
	}
	if (!isResolutionCorrect())
	{
		QMessageBox::critical(NULL, tr("Resolution Error"), tr("The resolution you sepcified is incorrect; it should consist of numbers only."));
		return;
	}
	*/

	// build the drive letter as a const char *
	//   (without the surrounding brackets)

	int select = m_comboDevice.GetCurSel();
	int volumeID = deviceInfo[select].volumeID;
	int deviceID = deviceInfo[select].deviceID;

	/*
	QString qs = cboxDevice->currentText();
	qs.replace(QRegExp("[\\[\\]]"), "");
	QByteArray qba = qs.toLocal8Bit();
	char *ltr = qba.data();
	*/
	char *ltr = deviceInfo[select].deviceName;

	CString path;
	GetDlgItemText(IDC_MFCEDIT_PWD, path);

	if (!path.IsEmpty())
	{
		CFile file;
		unsigned long long filesize = 0;
		if (file.Open(path, CFile::modeRead))
		{
			filesize = file.GetLength();
			file.Close();
		}

		if (filesize > 0)
		{
			/*
			if (leFile->text().at(0) == cboxDevice->currentText().at(1))
			{
				QMessageBox::critical(NULL, tr("Write Error"), tr("Image file cannot be located on the target device."));
				return;
			}
			*/
			/*
			if (QMessageBox::warning(NULL, tr("Confirm overwrite"), tr("Writing to a physical device can corrupt the device.\n"
				"(Target Device: %1 \"%2\")\n"
				"Are you sure you want to continue?").arg(cboxDevice->currentText()).arg(getDriveLabel(ltr)),
				QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
			{
				return;
			}
			*/
			/*
			bCancel->setEnabled(true);
			bWrite->setEnabled(false);
			bRead->setEnabled(false);
			*/
			double mbpersec;
			unsigned long long i, lasti, availablesectors, numsectors;

			hVolume = cdisk.getHandleOnVolume(volumeID, GENERIC_WRITE);
			if (hVolume == INVALID_HANDLE_VALUE)
			{
				TRACE("%s INVALID_HANDLE_VALUE\n", __func__);
				status = STATUS_IDLE;
				SetDisableDlg();
				return;
			}
			if (!cdisk.getLockOnVolume(hVolume))
			{
				TRACE("%s getLockOnVolume\n", __func__);
				CloseHandle(hVolume);
				status = STATUS_IDLE;
				SetDisableDlg();
				return;
			}
			if (!cdisk.unmountVolume(hVolume))
			{
				TRACE("%s unmountVolume\n", __func__);
				cdisk.removeLockOnVolume(hVolume);
				CloseHandle(hVolume);
				status = STATUS_IDLE;
				SetDisableDlg();
				return;
			}
			hFile = cdisk.getHandleOnFile((LPSTR)(LPCTSTR(path)), GENERIC_READ);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				TRACE("%s getHandleOnFile\n", __func__);
				cdisk.removeLockOnVolume(hVolume);
				CloseHandle(hVolume);
				status = STATUS_IDLE;
				SetDisableDlg();
				return;
			}
			hRawDisk = cdisk.getHandleOnDevice(deviceID, GENERIC_WRITE);
			if (hRawDisk == INVALID_HANDLE_VALUE)
			{
				TRACE("%s getHandleOnDevice\n", __func__);
				cdisk.removeLockOnVolume(hVolume);
				CloseHandle(hVolume);
				CloseHandle(hFile);
				status = STATUS_IDLE;
				SetDisableDlg();
				return;
			}
			availablesectors = cdisk.getNumberOfSectors(hRawDisk, &sectorsize);
			numsectors = cdisk.getFileSizeInSectors(hFile, sectorsize);
			if (numsectors > availablesectors)
			{
				CString msg;
				msg.Format(_T("Not enough space on disk : Size: %d sectors  Available : %d sectors  Sector size : %d"), numsectors, availablesectors, sectorsize);
				MessageBox(msg, _T("Write Error"), MB_ICONEXCLAMATION | MB_ICONWARNING);
				//QMessageBox::critical(NULL, tr("Write Error"), tr("Not enough space on disk: Size: %1 sectors  Available: %2 sectors  Sector size: %3").arg(numsectors).arg(availablesectors).arg(sectorsize));
				cdisk.removeLockOnVolume(hVolume);
				CloseHandle(hVolume);
				CloseHandle(hFile);
				CloseHandle(hRawDisk);
				status = STATUS_IDLE;
				SetDisableDlg();
				return;
			}

			//progressbar->setRange(0, (numsectors == 0ul) ? 100 : (int)numsectors);
			m_ProgressBar.SetRange(0, 1000);
			lasti = 0ul;
			//timer.start();
			timer_cnt = 0;
			ull_maxTime = numsectors;
			SetTimer(USER_TIMER, TIMER_TIME, NULL);
			for (i = 0ul; i < numsectors && status == STATUS_WRITING; i += 1024ul)
			{
				sectorData = cdisk.readSectorDataFromHandle(hFile, i, (numsectors - i >= 1024ul) ? 1024ul : (numsectors - i), sectorsize);
				if (sectorData == NULL)
				{
					TRACE("%s readSectorDataFromHandle\n", __func__);
					delete sectorData;
					sectorData = NULL;
					cdisk.removeLockOnVolume(hVolume);
					CloseHandle(hVolume);
					CloseHandle(hFile);
					CloseHandle(hRawDisk);
					status = STATUS_IDLE;
					KillTimer(USER_TIMER);
					SetDisableDlg();
					return;
				}
				if (!cdisk.writeSectorDataToHandle(hRawDisk, sectorData, i, (numsectors - i >= 1024ul) ? 1024ul : (numsectors - i), sectorsize))
				{
					TRACE("%s writeSectorDataToHandle\n", __func__);
					delete sectorData;
					sectorData = NULL;
					cdisk.removeLockOnVolume(hVolume);
					CloseHandle(hVolume);
					CloseHandle(hFile);
					CloseHandle(hRawDisk);
					status = STATUS_IDLE;
					KillTimer(USER_TIMER);
					SetDisableDlg();
					return;
				}
				delete sectorData;
				sectorData = NULL;
				/*
				QCoreApplication::processEvents();
				if (timer.elapsed() >= 1000)
				{
					mbpersec = (((double)sectorsize * (i - lasti)) * (1000.0 / timer.elapsed())) / 1024.0 / 1024.0;
					statusbar->showMessage(QString("%1MB/s").arg(mbpersec));
					timer.start();
					lasti = i;
				}
				progressbar->setValue(i);
				QCoreApplication::processEvents();
				*/
				m_ProgressBar.SetPos((int)(i * 1000 / numsectors));
				ull_userTime = i;
			}
			cdisk.removeLockOnVolume(hVolume);
			CloseHandle(hVolume);
			CloseHandle(hFile);
			CloseHandle(hRawDisk);
			KillTimer(USER_TIMER);
			sectorData = NULL;
			if (status == STATUS_CANCELED)
			{
				passfail = false;
			}
		}
		/*
		else if (!fileinfo.exists() || !fileinfo.isFile())
		{
			QMessageBox::critical(NULL, tr("File Error"), tr("The selected file does not exist."));
			passfail = false;
		}
		else if (!fileinfo.isReadable())
		{
			QMessageBox::critical(NULL, tr("File Error"), tr("You do not have permision to read the selected file."));
			passfail = false;
		}
		*/
		else if (filesize == 0)
		{
			MessageBox(_T("The specified file contains no data."), _T("File Error"), MB_ICONEXCLAMATION | MB_ICONWARNING);
			//QMessageBox::critical(NULL, tr("File Error"), tr("The specified file contains no data."));
			passfail = false;
		}
		/*
		progressbar->reset();
		statusbar->showMessage(tr("Done."));
		bCancel->setEnabled(false);
		setReadWriteButtonState();
		*/
		if (passfail)
		{
			MessageBox(_T("Writing of image successful."), _T("Complete"), MB_ICONEXCLAMATION | MB_ICONWARNING);
			//QMessageBox::information(NULL, tr("Complete"), tr("Writing of image successful."));
		}
		else
		{
			MessageBox(_T("Writing Canceled."), _T("Complete"), MB_ICONEXCLAMATION | MB_ICONWARNING);
		}

	}
	/*
	 * Configuration of OS-startup by editing files written on USB device
	 *
	 */
	 /*
	 if (configurationShouldBeWritten())
	 {
		 if (passfail)
		 {
			 passfail = writeOSConfiguration(ltr);
			 if (passfail)
			 {
				 QMessageBox::information(NULL, tr("Complete"), tr("Writing configuration data successful."));
			 }
		 }
		 else
		 {
			 QMessageBox::critical(NULL, tr("Error"), tr("Configuration data not written because of previously occurred error(s)."));
		 }
	 }
	 if (status == STATUS_EXIT)
	 {
		 close();
	 }
	 */
	status = STATUS_IDLE;
	SetDisableDlg();
}


/*
 * ON_WM_DEVICECHANGE 헨들러 호출 함수
 */
BOOL CWin32DiskImagerDlg::OnDeviceChange(UINT nEventType, DWORD dwdata)
{
	getLogicalDrives();
	return TRUE;
}


void CWin32DiskImagerDlg::OnChangeMfceditPwd()
{
	SetDisableDlg();
}


void CWin32DiskImagerDlg::OnTimer(UINT_PTR nIDEvent)
{
	float _time, _ing, _max_time;
	CString text;

	switch (nIDEvent)
	{
	case USER_TIMER:
		timer_cnt++;

		_time = float(float(float(TIMER_TIME) / 1000) * timer_cnt);
		_ing = float(float(float(ull_userTime) / float(ull_maxTime)) * 100);
		_max_time = _time * 100 / _ing;
//		TRACE("%lf / %lf\n", _time, _max_time);
		text.Format(_T("%.02lf / %.02lf(s)"), _time, _max_time);
		SetDlgItemText(IDC_STATIC_LOG, text);
		break;
	default:
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}
