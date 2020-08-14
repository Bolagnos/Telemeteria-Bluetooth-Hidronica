/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package GPRS_Server_1920;

/**
 *
 * @author Administrator
 */
import java.io.File;
import java.io.FileWriter;
        
public class CFile {
    
    File m_file;
    FileWriter wr_file;
    
    CFile()
    {

    }
    
    File createFile(String nameFile, String infoReport)
    {
        this.m_file=new File(nameFile+".txt");
        
        try
        {
        if(this.m_file.createNewFile())
        {
            System.out.println("File successfully create.\n");
            this.m_file.setWritable(true);
            this.m_file.setReadable(true);
            this.wr_file=new FileWriter(this.m_file);
            this.wr_file.write(infoReport);
            this.wr_file.close();
        }
        else
        {
            System.out.println("The file requested wasn's created.\n");
        }
        }
        catch(Exception ex)
        {
            System.out.println("Error trying to create the file.\n");
        }
        return this.m_file;
    }
}
