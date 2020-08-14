    /*
 * GPRSMultiServerThread.java
 *
 * Created on July 31, 2006, 5:37 PM
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
import java.util.Date;
import java.util.*;
import java.text.SimpleDateFormat;
import java.text.DecimalFormat;
import java.util.GregorianCalendar;
import java.net.Socket;
import java.util.Vector;
import java.lang.Float;
import java.lang.Integer;

public class CServidorTelGPRS1920 extends Thread 
{
    private Vector mVector;
    private Socket mySocket = null;
    private CHandlerDB2 mySQLDriver=null; 
    
    private String strRegs []= {"HGPRSDATA:","# Serie: ","I1: ","V1: "};

    private String myMessage;              //  variable de almacenamiento de info a enviar via SMS
    private String myNoSerieEQ;
    private CMail m_MailHandler;
    
    private String AT600_VelocityU[]={"Velocity","m/s","Ft/s",};

    private String AT600_VolumetricU[]={"Volumetric",
                           "A-F/D","A-F/H","A-F/M","A-F/S","A-I/D","A-I/H","A-I/M","A-I/S","Mf^3/d","ft^3/h",
                           "ft^3/m","ft^3/s","MGD","GAL/H","GAL/M","GAL/S","MBL/D","BBL/D","BBL/H","BBL/M",
                           "BBL/S","Mm^3/d","m^3/h","m^3/m","m^3/s","ML/D","L/H","L/M","L/S","m^3/d",
                           "L/d","ft^3/d","Gal/d","KGal/m","KGal/h","KGal/d","Sm^3/s","Sm^3/m","Sm^3/h","Sm^3/d",
                           "SL/s","SL/m","SL/h","SML/d","SCFM","SCFH"
                          };
    private String AT600_TotalizerU[]={"FWD Total",
                          "ML","Mm^3","MBBL","BBL","MGAL","GAL","Mft^3","ft^3","AC-IN","AC-FT",
                          "m^3","L","Sm^3","SL","kg","t","SFt^3","LB","St"
                          };
    /**********************************************************************************/
    public CServidorTelGPRS1920()
    {
    }


    public CServidorTelGPRS1920(Socket socket)
    {
        super("CServidorTelGPRS1920");
        this.myMessage="Reporte de equipo:\r\n";
        //this.mVector=pVector;
        this.mySocket = socket;
        mySQLDriver=new CHandlerDB2();
         //this.destroy();
        m_MailHandler=new CMail();
    }
/**********************************************************************************/            
    public void run() 
    {
            this.capturaMensajeGPRS();
           // this.registerDBData();
    }
    
/**********************************************************************************/                
    private void capturaMensajeGPRS()
    {
        String mydatestr="";
        String ReturnMessage="";
        String DNS_Name="";
        String IP_Number="";
        
	try
        {
	    PrintWriter out = new PrintWriter(mySocket.getOutputStream(), true);
	    BufferedReader in = new BufferedReader(
				    new InputStreamReader(
				    mySocket.getInputStream()));

            String inputLine, outputLine;
            String myDailyReportHour;
            Date mydate,mydate1;
            Calendar myCal1;
            
            int k=0;
            int u=0;
            
            this.myMessage="";

            String [] strLine2;
            String [] strVars2;//=new String [15];

            while ((inputLine = in.readLine()) != null)
            {
               // System.out.println("DBG:"+inputLine); // Linea de debbug

                if(inputLine.startsWith("END BLOCK"))
                {
                   // this.destroy();

                     SimpleDateFormat myFdate1=new SimpleDateFormat("Z");
                     String strTZ;
                     mydate1=new Date(System.currentTimeMillis());
                     strTZ=myFdate1.format(mydate1);
                     u=Integer.parseInt(strTZ);
                     u/=100;            // Obtienen el offset de la zona horaria local del servidor


                    myCal1=Calendar.getInstance();
                    myCal1.add(Calendar.HOUR,((-1)*u));  // Calcula horario GMT a partir del Time Zone en el servidor

                
                    SimpleDateFormat myFdate=new SimpleDateFormat("ddMMyyyyHHmmssZ");
                    int p_Str1;


                    // Lineas para establecer el horario que sera pasado al equipo de telemetria remotamente de acuerdo a su Time Zone
                    try
                    {
                        strLine2=myMessage.split("\r\n");
                        for(u=0;u<strLine2.length;u++)
                        {
                            if(strLine2[u].indexOf("HGPRSDATA:")!=-1)
                            {
                                strVars2=strLine2[u].split(",");
                                
                                 //mydate1=new Date(System.currentTimeMillis());
                                //strTZ=myFdate1.format(mydate1);

                                 p_Str1=myMessage.indexOf("HGPRSDATA:");
                                 p_Str1+=10;
                                 switch (myMessage.charAt(p_Str1))
                                 {
                                    case '2':
                                                u=Integer.parseInt(strVars2[10]);
                                                break;
                                    case '1':
                                                u=Integer.parseInt(strVars2[2]);
                                                break;
                                 }

                                
                                //timeZone/=100;
                                myCal1.add(Calendar.HOUR,u); // Establece el horario a partir del Time Zone  en la telemetria y del horario GMT calculado a partir del servidor
                                mydatestr=myFdate.format(myCal1.getTime()); // Hora actual del servidor

                                //


                                ReturnMessage+="ADT: "+mydatestr+"\r\nEND SESSION\r\n";
                                //ReturnMessage+="HWRD:E\r\n";
                                out.println(ReturnMessage);
                                break;
                            }
                        }
                                               
                    }
                    catch(Exception ex3)
                    {
                        System.out.println(ex3.toString());
                    }
                    break;
                }
                else
                {
                    k=inputLine.indexOf(strRegs[1]); // Detecta linea recibida con el numero de serie
                    if(k!=-1)
                    {
                        k+=strRegs[1].length();
                    }
                    
                    myMessage=myMessage.concat(inputLine);
                    myMessage=myMessage.concat("\r\n");
                }
                
	    }
                        
	    out.close();
	    in.close();
	    mySocket.close();

            
	}
    catch (IOException e)
    {
         System.err.println("Error de conexion");
         e.printStackTrace();
    }
        
            
            myMessage=myMessage.concat(mydatestr);
            myMessage=myMessage.concat("\r\n");
            System.out.println("GPRS Report:\r\n"+myMessage+"\r");
            System.out.println("Response:\r\n"+ReturnMessage+"\r");
            
            int p_Str=myMessage.indexOf("HGPRSDATA:");
            p_Str+=10;
            switch (myMessage.charAt(p_Str))
            {
                case '2': // Detecta cuando se trata de un mensaje de reporte
                         //p_Str=myMessage.indexOf(",",p_Str);
                         //p_Str+=1;
                         String idTelemetria="";
                         int id_Product=0;
                         int timeZone=0;
                         String str_Report_Formated="";
                         int i=0;
                         int n=0;
                         String [] strLine=myMessage.split("\r\n");
                         String [] strVars;//=new String [15];
                         String strplaceID="";
                         String strreportID="";
                         double v_gasto=0;
                         Calendar m_date=Calendar.getInstance();
                         String [] str_date = new String[3];
                         String [] str_hour = new String[3];
                         String ID_Me_or_SI = null;
                         String RFC = null;
                         String LogicNSM = null; // Numero de serie lógico del medidor
                         String NSM = null; // Numero de serie externo del medidor
                         String NSUE = null; // Numero de serie de la unidad electrónica (Telemetría-logger)
                         String posLatitude = null;
                         String posLongitude = null;
                         String Kerr = null;
                         String [] Totalizer_IntAndFactPart = new String[2];
                         //SimpleDateFormat m_fDate= new SimpleDateFormat("*dd/MM/yy,HH:mm:ss,");
                         SimpleDateFormat m_fDate= new SimpleDateFormat("yyyyMMdd|HHmmss|");
                         String m_DateStr;

                         DecimalFormat fn_instance=new DecimalFormat("#######");
                         fn_instance.applyPattern("#0.000");

                         //Date mydate1;
                         //SimpleDateFormat myFdate1=new SimpleDateFormat("Z");
                         //String strTZ;

                         for(int x=0;x<strLine.length-1;x++)
                         {
                            strVars=strLine[x].split(",");
                            n=0;
                            if(x==0) // Es la primera línea de los registros enviados?
                            {
                                idTelemetria=strVars[1];
                                strplaceID=this.mySQLDriver.getTubeID(idTelemetria);
                                strreportID=this.mySQLDriver.getIDVarTypetoReporte(Integer.parseInt(strVars[9]));
                                id_Product=Integer.parseInt(strVars[1].substring(0, 2));

                                NSUE = strVars[1]; // Numero de serie de la unidad electrónica (Telemetría-logger)
                                ID_Me_or_SI = strVars[3];
                                RFC = strVars[4];
                                LogicNSM = strVars[5]; // Numero de serie lógico del medidor
                                NSM = strVars[6]; // Numero de serie del medidor externo
                                posLatitude = strVars[7];
                                posLongitude = strVars[8];
                                Kerr = "000";

                                //mydate1=new Date(System.currentTimeMillis());
                                //strTZ=myFdate1.format(mydate1);
                                //timeZone=Integer.parseInt(strTZ);
                                //timeZone/=100;
                                //timeZone=Integer.parseInt(strVars[10]); // Establece el timeZone para sumarlo despues y obtener el horario UMT
                                timeZone=0;
                                n=11;
                                if(id_Product==30) // Solo para Telemetrias para medidores MPG2 Batt con memoria SD y bluetooth
                                {
                                    //str_date=strVars[n++].split("/");
                                    //str_hour=strVars[n++].split(":");
                                   //str_date = new String[3];
                                   str_date[0]=strVars[n].substring(0, 4); // YYYY
                                   str_date[1]=strVars[n].substring(4, 6); // MM
                                   str_date[2]=strVars[n].substring(6);    // DD
                                   n++;
                                   
                                   //str_hour= new String[3];
                                   str_hour[0]=strVars[n].substring(0, 2); // hh
                                   str_hour[1]=strVars[n].substring(2, 4); // mm
                                   str_hour[2]=strVars[n].substring(4);    // ss
                                   n++;
                                   
                                    m_date.set(Integer.parseInt(str_date[0]),Integer.parseInt(str_date[1])-1,Integer.parseInt(str_date[2]),Integer.parseInt(str_hour[0]),Integer.parseInt(str_hour[1]),Integer.parseInt(str_hour[2]));
                                    //m_date.add(Calendar.MILLISECOND,-m_date.get(m_date.ZONE_OFFSET)); // establece horario UMT
                                    m_date.add(Calendar.HOUR_OF_DAY,(-1)*(timeZone)); // establece horario UMT
                                    m_DateStr=m_fDate.format(m_date.getTime());
                                    //str_Report_Formated+=m_DateStr+strplaceID+","+strreportID+",";
                                    str_Report_Formated+=ID_Me_or_SI+"|"+m_DateStr+RFC+"|"+NSM+"|"+NSUE+"|";
                                }
                            }
                            else
                            {
                                //str_date=strVars[n++].split("/");
                                //str_hour=strVars[n++].split(":");
                                   str_date[0]=strVars[n].substring(0, 4); // YYYY
                                   str_date[1]=strVars[n].substring(4, 6); // MM
                                   str_date[2]=strVars[n].substring(6);    // DD
                                   n++;
                                   
                                   str_hour[0]=strVars[n].substring(0, 2); // hh
                                   str_hour[1]=strVars[n].substring(2, 4); // mm
                                   str_hour[2]=strVars[n].substring(4);    // ss
                                   n++;
                                
                                m_date.set(Integer.parseInt(str_date[0]),Integer.parseInt(str_date[1])-1,Integer.parseInt(str_date[2]),Integer.parseInt(str_hour[0]),Integer.parseInt(str_hour[1]),Integer.parseInt(str_hour[2]));
                                //m_date.add(Calendar.MILLISECOND,-m_date.get(m_date.ZONE_OFFSET)); // establece horario UMT
                                m_date.add(Calendar.HOUR_OF_DAY,(-1)*(timeZone)); // establece horario UMT
                                m_DateStr=m_fDate.format(m_date.getTime());
                                
                                //str_Report_Formated+=m_DateStr+strplaceID+","+strreportID+",";
                                str_Report_Formated+=ID_Me_or_SI+"|"+m_DateStr+RFC+"|"+NSM+"|"+NSUE+"|";
                            }

                          
                            switch(id_Product)
                             {
                                 
                                 case 30:// Telemetria MPL G2 Batt (ID_tipo de reporte, Número serie medidor MPG2, Unidades de medida, Gasto=lps, Volumen=m^3, Estado Puerta, Voltaje Batt en divisor)
                                           //str_Report_Formated+=strVars[n]+","+strVars[n+1]+","+strVars[n+2]+","+strVars[n+3]+","+strVars[n+4].substring(0,7)+"."+strVars[n+4].substring(7,12)+","+strVars[n+5]+","+strVars[n+6]+"\r\n";
                                            i=strVars[n+2].indexOf(".");
                                            Totalizer_IntAndFactPart[0] = strVars[n+2].substring(0, i);
                                            Totalizer_IntAndFactPart[1] = strVars[n+2].substring(i);
                                            str_Report_Formated+=Totalizer_IntAndFactPart[0]+"|"+posLatitude+"|"+posLongitude+"|"+Kerr+"\r\n";
                                           break;
                             
                                 default: break;
                             }

                            for(int y=0;y<strVars.length;y++)
                            {
                                strVars[y]="";
                            }
                         }

                         DNS_Name=this.mySQLDriver.getDNSName(idTelemetria);
                         if(DNS_Name!=null)
                         {
                            IP_Number=this.mySQLDriver.getDNSIPNumber(idTelemetria);
                            if(IP_Number!=null)
                            {
                                try
                                {
                                    this.sendReport_Via_TCP_IP(str_Report_Formated, DNS_Name, IP_Number);
                                }
                                catch(Exception ex1)
                                {
                                    System.out.println(ex1.toString());
                                }
                            }
                         }

                         this.sendMail(idTelemetria, str_Report_Formated);

                         break;
                case '1': // Detecta cuando se trata de un mensaje de inicio de sistema
                         break;
                default: break;
            }
            
    }
/**********************************************************************************/            
    private void registerDBData()
    {
        
            boolean RegisterSQL=false;
            int k,m=0,x;
            String [] mySQLInfo=new String [8];

                for(x=0;x<strRegs.length;x++)
                {
                    k=myMessage.indexOf(strRegs[x]);
                    k+=strRegs[x].length();
                    m=myMessage.indexOf('\r',k);
                    mySQLInfo[x]=myMessage.substring(k,m);
                    if(x==1)
                    {
                        myNoSerieEQ=mySQLInfo[x];
                        if(mySQLInfo[0].equals("01"))
                        {x=strRegs.length; break;}
                        if(mySQLInfo[0].equals("02"))
                        { RegisterSQL=true;}
                        if(mySQLInfo[0].equals("03"))
                        {RegisterSQL=true;}
                    }                    
                }
                
                k=m+2;
                m=myMessage.indexOf('\r',k);
                mySQLInfo[x]=myMessage.substring(k,m);
                
                if(RegisterSQL)
                {
                    //mySQLDriver.alta_Datos(mySQLInfo);
                }
        
        }
/**********************************************************************************/            
    
private void sendMail(String IDTelemetria, String Msg)
{
    try
    {
        String dest=this.mySQLDriver.getInfoEmailDest(IDTelemetria);
        String dest_cc=this.mySQLDriver.getInfoEmailDest_CC(IDTelemetria);
        String from=this.mySQLDriver.getInfoEmailRemitent(IDTelemetria);
        String frmpsw=this.mySQLDriver.getInfoEmailPSWRemitent(IDTelemetria);
        String host="smtp.gmail.com";
        String subject=this.mySQLDriver.getTituloCorreo(IDTelemetria);
    
        this.m_MailHandler.sendMailMessage(dest, dest_cc, from, frmpsw, host, subject, Msg);
    }
    catch(Exception ex)
    {
        System.out.println(ex.toString());
    }

}
/**********************************************************************************/
public void sendReport_Via_TCP_IP(String Msg, String DNS_Name, String DNS_IP_No) throws IOException
{
    Socket m_ClientSocket = null;
    InputStream mInputStr = null;
    OutputStream mOutputStr = null;
    BufferedReader in = null;
    String [] mStrCmp=Msg.split("\r\n");
    
    try
    {
        mStrCmp[(mStrCmp.length)-1]=mStrCmp[(mStrCmp.length)-1].replace('*','-');
        String ReturnedStrm="";
        String RdLine="";
        int ipNumber=Integer.parseInt(DNS_IP_No);
        
        m_ClientSocket=new Socket(DNS_Name,ipNumber);
        mInputStr=m_ClientSocket.getInputStream();
        mOutputStr=m_ClientSocket.getOutputStream();

        //PrintWriter out = new PrintWriter(mOutputStr, true);
	    in = new BufferedReader(new InputStreamReader(mInputStr));

         byte [] GPRSMsgByte = Msg.getBytes();
         mOutputStr.write(GPRSMsgByte);

         System.out.println("Message sent to LeesLab Server:\r\n"+Msg+"\r\n");

         while((RdLine=in.readLine())!=null)
         {
             ReturnedStrm+=RdLine+"\r\n";
             RdLine=RdLine.replace('*', '-');
             if(mStrCmp[(mStrCmp.length)-1].matches(RdLine))
             {
                break;
             }
         }

         mOutputStr.close();
         in.close();
         mInputStr.close();
         m_ClientSocket.close();


         System.out.println("LeesLab Server response:\r\n"+ReturnedStrm+"\r\n");
        
    }
    catch(Exception ex)
    {
        mOutputStr.close();
        in.close();
        mInputStr.close();
        m_ClientSocket.close();

        System.out.println("Socket Server Error: "+ex.toString());
    }


}
/**********************************************************************************/                
}

