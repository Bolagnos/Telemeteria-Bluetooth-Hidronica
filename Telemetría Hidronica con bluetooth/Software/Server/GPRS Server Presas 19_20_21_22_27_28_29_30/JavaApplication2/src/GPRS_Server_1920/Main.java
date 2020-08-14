/*
 * Main.java
 *
 * Created on July 31, 2006, 4:31 PM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */
package GPRS_Server_1920;

/**
 *
 * @author Bolagnos
 */
import java.net.*;
import java.io.*;

public class Main {

    /** Creates a new instance of Main */
    public Main() {
    }

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) throws IOException {
        ServerSocket serverSocket = null;
        boolean listen = true;
        CHandlerDB2 m_DB = new CHandlerDB2();
        
        File myFile = new CFile().createFile("TestFile","Hello groundhog!!!.\n");
        //myFile.createFile("EdithQG","Hello groundhog!!!.\n");
        CFTPClient myFT = new CFTPClient("localhost");
/****************************************/
/* Lineas de prueba conecion a servidor de datos externo*/

/*       CServidorTelGPRS1920 serverTest= new CServidorTelGPRS1920();
       serverTest.sendReport_Via_TCP_IP("*16/01/14,18:15:00,600,7,0.0000,0.0080,0.0213,0.0","dr025.leeslab.com.mx","00600");
 * */
/****************************************/

        try {
            serverSocket = new ServerSocket(2009);
        } catch (IOException e) {
            System.err.println("Could not listen on port: 2009.");
            System.exit(-1);
        }


        while (listen) {
            new CServidorTelGPRS1920(serverSocket.accept()).start();
        }

        serverSocket.close();

    }
}
