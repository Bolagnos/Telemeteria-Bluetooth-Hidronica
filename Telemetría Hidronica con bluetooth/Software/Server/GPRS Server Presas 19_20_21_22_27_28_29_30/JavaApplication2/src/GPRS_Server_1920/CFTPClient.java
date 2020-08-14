/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package GPRS_Server_1920;

import org.apache.commons.net.ftp.FTP;
import org.apache.commons.net.ftp.FTPClient;
import org.apache.commons.net.ftp.FTPHTTPClient;
import org.apache.commons.net.ftp.FTPClientConfig;
import org.apache.commons.net.ftp.FTPConnectionClosedException;
import org.apache.commons.net.ftp.FTPFile;
import org.apache.commons.net.ftp.FTPReply;
import org.apache.commons.net.ftp.FTPSClient;
import org.apache.commons.net.io.CopyStreamEvent;
import org.apache.commons.net.io.CopyStreamListener;
import org.apache.commons.net.util.TrustManagerUtils;

/**
 *
 * @author Administrator
 */
public class CFTPClient {
    
    String ftp_DNS_Name;
    FTPClient m_ftpClient;
            
    CFTPClient(String DNS_Name)
    {
        this.ftp_DNS_Name= DNS_Name;
        
        m_ftpClient = new FTPClient();
        try
        {
            m_ftpClient.connect(DNS_Name,21);
            System.out.println(m_ftpClient.getReplyString());
            m_ftpClient.login("Administrator", "EdithQG");
        
            int reply=m_ftpClient.getReplyCode();
            if(FTPReply.isPositiveCompletion(reply))
            {
                System.out.println("Connected to FTP Server.\n");
            }
            else
            {
                System.out.println("Not connected to FTP Server.\n");
            }
        
            m_ftpClient.disconnect();
        }
        catch (Exception ex)
        {
            System.out.println(ex.toString());
            try{
            m_ftpClient.disconnect();}
            catch(Exception ex1)
            {
                System.out.println("Could not disconnect from FTP Server\n");
            }
        }
    }
    
}
