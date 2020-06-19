Param(
    [Parameter(Mandatory=$true)]$Subject,
    [Parameter(Mandatory=$true)]$Body,
    [Parameter(Mandatory=$true)]$Attachment
);
$To = "xxxxxx@gmail.com";
$From = "xxxxxx@gmail.com";
$Password = "xxxxxxx";
$SMTPServer = "smtp.gmail.com";
$SMTPPort = "587";
[string]$userName = $From.Split("@")[0];
[securestring]$secStringPassword = ConvertTo-SecureString $Password -AsPlainText -Force;
[pscredential]$credOject = New-Object System.Management.Automation.PSCredential ($userName, $secStringPassword);
Send-MailMessage -From $From -To $To -Subject $Subject -Body $Body -SmtpServer $SMTPServer -port $SMTPPort -UseSsl -Credential $credOject -Attachments $Attachment;
