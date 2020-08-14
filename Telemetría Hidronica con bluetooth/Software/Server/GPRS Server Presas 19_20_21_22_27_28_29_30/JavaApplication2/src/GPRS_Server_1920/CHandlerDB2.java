/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package GPRS_Server_1920;

/**
 *
 * @author Administrator
 */

import java.sql.*;
import java.*;
import java.text.*;
import java.util.Date;
import java.util.GregorianCalendar;

public class CHandlerDB2 {

private String driverName="org.postgresql.Driver";
private String url_access="jdbc:postgresql:";
private String dbURL="";
private String dbInitialName="postgres";

private String dbName="";
private String theUser="";	
private String thePwd="";


public CHandlerDB2()
{
    this.createUserDB2();
    if(this.createDB2())
    {
        this.fillStartCatalogs2();
    }
}

/*********************************************************************************/
public String getField(String strQuery)
{
    String results []= new String [1];
    ResultSet rs;
    Connection con;

	try
	{
		Class.forName(driverName);
		con=DriverManager.getConnection(dbURL,theUser,thePwd);
		try
		{
			Statement st=con.createStatement();
			rs=st.executeQuery(strQuery);
			if(rs==null)
			{
				results=null;
			}
			else
			{
			  int x=0;

			  boolean reg_exist=rs.next();
			  if(reg_exist==true)
			  {
			   while(reg_exist)
			   {
				results [x]=rs.getString(x+1);
				reg_exist=rs.next();
			   }
			  }
			  else
			  {
				results=null;
			  }
			}
			rs.close();
		}
		finally
		{
			con.close();

		}

	}
	catch(Exception e)
	{
		return null;
	}

	return results[0];
}

/*********************************************************************************/
//Ejecuta una sentencia SQL que no devuelve Campos.

public boolean executeSQL(String sentence)
{
	boolean response=false;

	Connection con;
	try
	{
		Class.forName(driverName);
		con=DriverManager.getConnection(dbURL,theUser,thePwd);
		try
		{
			Statement st=con.createStatement();
			response=st.execute(sentence);
            response=true;

		}
        catch(Exception e)
        {
            System.out.println(e.toString());
            response=false;
        }
		finally
		{
			con.close();
		}//try
	}
	catch(Exception e)
	{
            String errorMsg=e.toString();
            int noExistDB=errorMsg.indexOf("already exists");
            if(noExistDB==-1)
            {
                System.out.println(errorMsg);
            }
	}//try
	return response;
}

/*********************************************************************************/
private boolean createDB2()
{

    boolean result;
    dbURL=url_access+dbInitialName;
    dbName="dbpresasGPRS1920";
    theUser="dbadmin";
    thePwd="managerdb";

    String strsql="CREATE DATABASE \""+dbName+"\"  WITH OWNER = "+theUser+" ENCODING = 'UTF8';";
    result=this.executeSQL(strsql);

    if(result)
    {
        this.createDB2CatalogReportType();
        this.createDB2CatalogError();
        this.createDB2CatalogUser();
        this.createDB2CatalogMexStates();


        this.createDB2ModemGPRSTable();
        this.createDB2TelemetryTable();
        this.createDB2LocationTable();
        this.createDB2TubeTable();
        this.createDB2EmailReportTable();
        this.createDB2DNSClientTable();
        this.createDB2MeterTable();
        this.createDB2DataTable();
    }

    return result;
}

private boolean createUserDB2()
 {
    boolean result;
    dbURL=url_access+dbInitialName;
    theUser="postgres";
    thePwd="admpostgres";

    String strsql="CREATE ROLE dbadmin LOGIN PASSWORD 'managerdb' SUPERUSER INHERIT CREATEDB CREATEROLE  VALID UNTIL 'infinity';";
    result=this.executeSQL(strsql);

    return result;
 }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//          Inicio de lineas para Base de Datos Telemetria GPRS para equipo con Itzy Flow, MPG2 y Milltornics OCM III Siemens
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

private boolean createDB2ModemGPRSTable()
{

    boolean result;
    dbName="dbpresasGPRS1920";
    dbURL=url_access+dbName;
    theUser="dbadmin";
    thePwd="managerdb";

    String strsql = "CREATE TABLE modemgprs" +
                    "(" +
                    "  id_modemgprs serial NOT NULL," +
                    "  modelo character(20)," +
                    "  marca character(20),"+
                    "  IMEI_number character(20),"+
                    "  CONSTRAINT pk_id_modemgprs PRIMARY KEY (id_modemgprs)" +
                    ")" +
                    "WITH (OIDS=FALSE);" +
                    "ALTER TABLE modemgprs OWNER TO "+theUser+";";

    result=this.executeSQL(strsql);
    return result;
}

private boolean createDB2TelemetryTable()
{

    boolean result;
    dbName="dbpresasGPRS1920";
    dbURL=url_access+dbName;
    theUser="dbadmin";
    thePwd="managerdb";

    String strsql = "CREATE TABLE telemetria" +
                    "(" +
                    "  id_telemetria character(12) NOT NULL," +
                    "  fk_id_modemgprs integer NOT NULL," +
                    "  modelo character(20)," +
                    "  tipo character(20),"+
                    "  id_producto integer,"+
                    "  time_zone integer,"+
                    "  active_state boolean DEFAULT true,"+
                    "  flag_enable_tel boolean DEFAULT false,"+
                    "  CONSTRAINT pk_id_telemetria PRIMARY KEY (id_telemetria)," +
                    "  CONSTRAINT fk_id_modemgprs FOREIGN KEY (fk_id_modemgprs)" +
                    "      REFERENCES modemgprs (id_modemgprs) MATCH SIMPLE" +
                    "      ON UPDATE CASCADE ON DELETE SET DEFAULT" +
                    ")" +
                    "WITH (OIDS=FALSE);" +
                    "ALTER TABLE telemetria OWNER TO "+theUser+";";

    result=this.executeSQL(strsql);
    return result;
}

private boolean createDB2MeterTable()
{

    boolean result;
    dbName="dbpresasGPRS1920";
    dbURL=url_access+dbName;
    theUser="dbadmin";
    thePwd="managerdb";

    String strsql = "CREATE TABLE medidor" +
                    "(" +
                    "  id_medidor serial NOT NULL," +
                    "  fk_id_telemetria character(12) NOT NULL," +
                    "  fk_id_tubo character(10) NOT NULL," +
                    "  fk_id_reporte integer NOT NULL," +
                    "  fk_id_emailreport integer,"+
                    "  fk_id_dns_client integer,"+
                    "  modelo character(20)," +
                    "  tipo character(20),"+
                    "  no_serie character(20),"+
                    "  titulo_correo character(60),"+
                    "  CONSTRAINT pk_id_medidor PRIMARY KEY (id_medidor)," +
                     "  CONSTRAINT fk_id_telemetria FOREIGN KEY (fk_id_telemetria)" +
                    "      REFERENCES telemetria (id_telemetria) MATCH SIMPLE" +
                    "      ON UPDATE CASCADE ON DELETE SET DEFAULT," +
                    "  CONSTRAINT fk_id_tubo FOREIGN KEY (fk_id_tubo)" +
                    "      REFERENCES tubo (id_tubo) MATCH SIMPLE" +
                    "      ON UPDATE CASCADE ON DELETE SET DEFAULT," +
                    "  CONSTRAINT fk_id_reporte FOREIGN KEY (fk_id_reporte)" +
                    "      REFERENCES c_reporte (id_reporte) MATCH SIMPLE" +
                    "      ON UPDATE CASCADE ON DELETE SET DEFAULT," +
                    "  CONSTRAINT fk_id_emailreport FOREIGN KEY (fk_id_emailreport)" +
                    "      REFERENCES emailreport (id_emailreport) MATCH SIMPLE" +
                    "      ON UPDATE CASCADE ON DELETE SET DEFAULT," +
                     "  CONSTRAINT fk_id_dns_client FOREIGN KEY (fk_id_dns_client)" +
                    "      REFERENCES dns_client (id_dns_client) MATCH SIMPLE" +
                    "      ON UPDATE CASCADE ON DELETE SET DEFAULT" +
                    ")" +
                    "WITH (OIDS=FALSE);" +
                    "ALTER TABLE medidor OWNER TO "+theUser+";";

    result=this.executeSQL(strsql);
    return result;
}

private boolean createDB2EmailReportTable()
{

    boolean result;
    dbName="dbpresasGPRS1920";
    dbURL=url_access+dbName;
    theUser="dbadmin";
    thePwd="managerdb";


    String strsql = "CREATE TABLE emailreport" +
                    "(" +
                    "  id_emailreport serial NOT NULL," +
                    "  e_remitente character(30) NOT NULL," +
                    "  psw_remitente character(20) NOT NULL,"+
                    "  e_contact character(30) NOT NULL,"+
                    "  e_contact_cc character(180),"+
                    "  CONSTRAINT pk_id_emailreport PRIMARY KEY (id_emailreport)" +
                    ")" +
                    "WITH (OIDS=FALSE);" +
                    "ALTER TABLE emailreport OWNER TO "+theUser+";";

    result=this.executeSQL(strsql);
    return result;
}

private boolean createDB2DNSClientTable()
{

    boolean result;
    dbName="dbpresasGPRS1920";
    dbURL=url_access+dbName;
    theUser="dbadmin";
    thePwd="managerdb";

    String strsql = "CREATE TABLE dns_client" +
                    "(" +
                    "  id_dns_client serial NOT NULL," +
                    "  dns_ip character(60) NOT NULL," +
                    "  port_ip character(20) NOT NULL,"+
                    "  name_client character(60) NOT NULL,"+
                    "  CONSTRAINT pk_id_dns_client PRIMARY KEY (id_dns_client)" +
                    ")" +
                    "WITH (OIDS=FALSE);" +
                    "ALTER TABLE dns_client OWNER TO "+theUser+";";

    result=this.executeSQL(strsql);
    return result;
}

private boolean createDB2TubeTable()
{

    boolean result;
    dbName="dbpresasGPRS1920";
    dbURL=url_access+dbName;
    theUser="dbadmin";
    thePwd="managerdb";

    String strsql = "CREATE TABLE tubo" +
                    "(" +
                    "  id_tubo character(10) NOT NULL," +
                    "  fk_id_localizacion integer NOT NULL,"+
                    "  diametro character(10)," +
                    "  no_equipo character(10)," +
                    "  nombre_tubo character(10),"+
                    "  CONSTRAINT pk_id_tubo PRIMARY KEY (id_tubo)," +
                    "  CONSTRAINT fk_id_localizacion FOREIGN KEY (fk_id_localizacion)" +
                    "      REFERENCES localizacion (id_localizacion) MATCH SIMPLE" +
                    "      ON UPDATE CASCADE ON DELETE SET DEFAULT" +
                    ")" +
                    "WITH (OIDS=FALSE);" +
                    "ALTER TABLE tubo OWNER TO "+theUser+";";

    result=this.executeSQL(strsql);
    return result;
}

private boolean createDB2LocationTable()
{

    boolean result;
    dbName="dbpresasGPRS1920";
    dbURL=url_access+dbName;
    theUser="dbadmin";
    thePwd="managerdb";


    String strsql = "CREATE TABLE localizacion" +
                    "(" +
                    "  id_localizacion serial NOT NULL," +
                    "  fk_estado integer NOT NULL," +
                    "  direccion character(300) NOT NULL," +
                    "  municipio character(40)," +
                    "  CONSTRAINT pk_id_localizacion PRIMARY KEY (id_localizacion)," +
                    "  CONSTRAINT fk_estado_loc FOREIGN KEY (fk_estado)" +
                    "      REFERENCES c_estado_mex (id_estado_mex) MATCH SIMPLE" +
                    "      ON UPDATE CASCADE ON DELETE SET DEFAULT" +
                    ")" +
                    "WITH (OIDS=FALSE);" +
                    "ALTER TABLE localizacion OWNER TO "+theUser+";";

    result=this.executeSQL(strsql);
    return result;
}

private boolean createDB2CatalogReportType()
{

    boolean result;
    dbName="dbpresasGPRS1920";
    dbURL=url_access+dbName;
    theUser="dbadmin";
    thePwd="managerdb";

    String strsql="CREATE TABLE c_reporte" +
                  "(" +
                  "  id_reporte integer NOT NULL," +
                  "  Id_reportado integer," +
                  "  descripcion character(40) NOT NULL," +
                  "  CONSTRAINT pk_id_reporte PRIMARY KEY (id_reporte)" +
                  ")" +
                  "WITH (OIDS=FALSE);" +
                  "ALTER TABLE c_reporte OWNER TO "+theUser+";";

    result=this.executeSQL(strsql);
    return result;
}

private boolean createDB2CatalogError()
{

    boolean result;
    dbName="dbpresasGPRS1920";
    dbURL=url_access+dbName;
    theUser="dbadmin";
    thePwd="managerdb";

    String strsql="CREATE TABLE c_error" +
                  "(" +
                  "  id_error integer NOT NULL," +
                  "  descripcion character(40) NOT NULL," +
                  "  CONSTRAINT pk_id_error PRIMARY KEY (id_error)" +
                  ")" +
                  "WITH (OIDS=FALSE);" +
                  "ALTER TABLE c_error OWNER TO "+theUser+";";

    result=this.executeSQL(strsql);
    return result;
}

private boolean createDB2CatalogUser()
{

    boolean result;
    dbName="dbpresasGPRS1920";
    dbURL=url_access+dbName;
    theUser="dbadmin";
    thePwd="managerdb";

    String strsql="CREATE TABLE c_tipo_usuario"+
                  "("+
                  "  id_tipo_usuario integer NOT NULL," +
                  "  descripcion character (20) NOT NULL," +
                  "  CONSTRAINT pk_id_tipo_usuario PRIMARY KEY (id_tipo_usuario)" +
                  ")" +
                  "WITH (OIDS=FALSE);" +
                  "ALTER TABLE c_tipo_usuario OWNER TO "+theUser+";";


    result=this.executeSQL(strsql);
    return result;
}

private boolean createDB2CatalogMexStates()
{

    boolean result;
    dbName="dbpresasGPRS1920";
    dbURL=url_access+dbName;
    theUser="dbadmin";
    thePwd="managerdb";

    String strsql="CREATE TABLE c_estado_mex"+
                  "("+
                  "  id_estado_mex integer NOT NULL," +
                  "  nombre character (30) NOT NULL," +
                  "  CONSTRAINT pk_id_estado_mex PRIMARY KEY (id_estado_mex)" +
                  ")" +
                  "WITH (OIDS=FALSE);" +
                  "ALTER TABLE c_estado_mex OWNER TO "+theUser+";";


    result=this.executeSQL(strsql);
    return result;
}

private boolean createDB2DataTable()
{

    boolean result;
    dbName="dbpresasGPRS1920";
    dbURL=url_access+dbName;
    theUser="dbadmin";
    thePwd="managerdb";

    String strsql="CREATE TABLE registro"+
                  "("+
                  "  id_registro serial NOT NULL," +
                  "  fk_id_medidor integer," +
                  "  fk_error integer," +
                  "  fecha date NOT NULL," +
                  "  hora time without time zone NOT NULL," +
                  "  gasto character (15) NOT NULL," +
                  "  volumentot character(15) NOT NULL," +
                  "  nivel character(15) NOT NULL," +
                  "  velocidad_media character(15)," +
                  "  CONSTRAINT pk_id_registro PRIMARY KEY (id_registro)," +
                  "  CONSTRAINT fk_id_medidor FOREIGN KEY (fk_id_medidor)" +
                  "      REFERENCES medidor (id_medidor) MATCH SIMPLE" +
                  "      ON UPDATE CASCADE ON DELETE SET DEFAULT," +
                  "  CONSTRAINT fk_error FOREIGN KEY (fk_error)" +
                  "      REFERENCES c_error (id_error) MATCH SIMPLE" +
                  "      ON UPDATE CASCADE ON DELETE SET DEFAULT," +
                  "  CONSTRAINT uk_date_hour UNIQUE (fecha, hora)"+
                  ")" +
                  "WITH (OIDS=FALSE);" +
                  "ALTER TABLE registro OWNER TO "+theUser+";";


    result=this.executeSQL(strsql);
    return result;
}

private String [] reportTypeM2 = {"Gasto, Volumen, Nivel","Gasto, Volumen","Gasto, Nivel","Gasto","Volumen, Nivel","Volumen","Nivel","Gasto, Volumen, Nivel, Velocidad media","Gasto, Volumen, Velocidad media","Gasto, Nivel, Velocidad media","Gasto, Velocidad media","Volumen, Nivel, Velocidad media","Volumen, Velocidad media","Nivel, Velocidad media","velocidad media" };

private String [] errorTypeM2 = {"Ninguno","1","2","3","4","5","6","7","8","9","10","11","12"};
private String [] userTypeM2 = {"Administrador","Usuario"};
private String [] estadoMex2 = {"Aguascalientes","Baja California Nte.","Baja California Sur","Chihuahua","Colima","Campeche","Coahuila","Chiapas","Distrito  Federal","Durango","Guerrero","Guanajuato","Hidalgo","Jalisco","Michoacan","Morelos","Edo. de Mexico","Nayarit","Nuevo Leon","Oaxaca","Puebla","Quintana Roo","Queretaro","Sinaloa","San Luis Potosi","Sonora","Tabasco","Tlaxcala","Tamaulipas","Veracruz","Yucatan","Zacatecas"};


private void fillStartCatalogs2()
{
    int x;

    for(x=0;x<reportTypeM2.length;x++){
        this.alta_ReportType2(x, reportTypeM2[x]);
    }
    for(x=0;x<errorTypeM2.length;x++){
        this.alta_ErrorCatalog2(x, errorTypeM2[x]);
    }
    for(x=0;x<userTypeM2.length;x++){
        this.alta_TypeUserCatalog2(x, userTypeM2[x]);
    }
    for(x=0;x<estadoMex2.length;x++){
        this.alta_EstadoMexCatalog2(x, estadoMex2[x]);
    }

}

private boolean alta_ReportType2(int nReg, String description)
{
	boolean result;
    dbURL=url_access+dbName;
	//String strsql="insert into c_reporte (id_reporte, Id_reportado, descripcion) values ("+nReg+",'"+description+"')";
        String strsql="insert into c_reporte (id_reporte, Id_reportado, descripcion) values ("+nReg+","+nReg+",'"+description+"')";
	result=this.executeSQL(strsql);
	return result;
}

private boolean alta_ErrorCatalog2(int nReg, String description)
{
	boolean result;
    dbURL=url_access+dbName;
	String strsql="insert into c_error (id_error, descripcion) values ("+nReg+",'"+description+"')";
	result=this.executeSQL(strsql);
	return result;
}

private boolean alta_TypeUserCatalog2(int nReg, String description)
{
	boolean result;
    dbURL=url_access+dbName;
	String strsql="insert into c_tipo_usuario (id_tipo_usuario, descripcion) values ("+nReg+",'"+description+"')";
	result=this.executeSQL(strsql);
	return result;
}

private boolean alta_EstadoMexCatalog2(int nReg, String description)
{
	boolean result;
    dbURL=url_access+dbName;
	String strsql="insert into c_estado_mex (id_estado_mex, nombre) values ("+nReg+",'"+description+"')";
	result=this.executeSQL(strsql);
	return result;
}

////////////////////////////////////////////////
public String getTubeID(String IDTelemetria) //OK
{
	String strsql;
        dbURL=url_access+dbName;
        //strsql="select medidor.fk_id_tubo from medidor where medidor.fk_id_telemetria='"+IDTelemetria+"';";
        strsql="select medidor.fk_id_tubo from medidor, telemetria where telemetria.id_telemetria='"+IDTelemetria+"' and telemetria.id_telemetria=medidor.fk_id_telemetria;";
        //strsql="select medidor.fk_id_tubo from telemetria, medidor where telemetria.fk_id_modemsatelital='"+IDModem+"' and telemetria.id_telemetria = medidor.fk_id_telemetria;";
	return getField(strsql).trim();
}

public String getTituloCorreo(String IDTelemetria) //OK
{
	String strsql;
        dbURL=url_access+dbName;
        //strsql="select medidor.titulo_correo from medidor where medidor.fk_id_telemetria='"+IDTelemetria+"';";
        strsql="select medidor.titulo_correo from medidor, telemetria where telemetria.id_telemetria='"+IDTelemetria+"' and telemetria.id_telemetria=medidor.fk_id_telemetria;";
	return getField(strsql).trim();
}

public String getInfoEmailRemitent(String IDTelemetria) //OK
{
	String strsql;
        dbURL=url_access+dbName;
        strsql="select emailreport.e_remitente from medidor, emailreport where medidor.fk_id_telemetria='"+IDTelemetria+"' and emailreport.id_emailreport=medidor.fk_id_emailreport;";
        //strsql="select emailreport.e_remitente from telemetria, medidor, emailreport where telemetria.fk_id_modemsatelital='"+IDModem+"' and telemetria.id_telemetria = medidor.fk_id_telemetria and medidor.fk_id_emailreport = emailreport.id_emailreport;";
	return getField(strsql).trim();
}

public String getInfoEmailPSWRemitent(String IDTelemetria) //OK
{
	String strsql;
        dbURL=url_access+dbName;
        strsql="select emailreport.psw_remitente from medidor, emailreport where medidor.fk_id_telemetria='"+IDTelemetria+"' and emailreport.id_emailreport=medidor.fk_id_emailreport;";
        //strsql="select emailreport.psw_remitente from telemetria, medidor, emailreport where telemetria.fk_id_modemsatelital='"+IDModem+"' and telemetria.id_telemetria = medidor.fk_id_telemetria and medidor.fk_id_emailreport = emailreport.id_emailreport;";
	return getField(strsql).trim();
}

public String getInfoEmailDest(String IDTelemetria) //OK
{
	String strsql;
        dbURL=url_access+dbName;
        strsql="select emailreport.e_contact from medidor, emailreport where medidor.fk_id_telemetria='"+IDTelemetria+"' and emailreport.id_emailreport=medidor.fk_id_emailreport;";
//        strsql="select emailreport.e_contact from telemetria, medidor, emailreport where telemetria.fk_id_modemsatelital='"+IDModem+"' and telemetria.id_telemetria = medidor.fk_id_telemetria and medidor.fk_id_emailreport = emailreport.id_emailreport;";
	return getField(strsql).trim();
}

public String getInfoEmailDest_CC(String IDTelemetria) //OK
{
	String strsql;
        dbURL=url_access+dbName;
        strsql="select emailreport.e_contact_cc from medidor, emailreport where medidor.fk_id_telemetria='"+IDTelemetria+"' and emailreport.id_emailreport=medidor.fk_id_emailreport;";
        //strsql="select emailreport.e_contact_cc from telemetria, medidor, emailreport where telemetria.fk_id_modemsatelital='"+IDModem+"' and telemetria.id_telemetria = medidor.fk_id_telemetria and medidor.fk_id_emailreport = emailreport.id_emailreport;";
	return getField(strsql).trim();
}

public String getReportID(String IDTelemetria) //OK
{
    String strsql;
        dbURL=url_access+dbName;
        strsql="select medidor.fk_id_reporte from medidor, telemetria where telemetria.id_telemetria='"+IDTelemetria+"' and telemetria.id_telemetria=medidor.fk_id_telemetria;";
        //strsql="select medidor.fk_id_reporte from telemetria, medidor where telemetria.fk_id_modemsatelital='"+IDModem+"' and telemetria.id_telemetria = medidor.fk_id_telemetria;";
	return getField(strsql).trim();
    
}

public String getIDVarTypetoReporte(int indexReport)
{
    String strsql;
        dbURL=url_access+dbName;
        strsql="select c_reporte.id_reportado from c_reporte where c_reporte.id_reporte="+indexReport+";";
	return getField(strsql).trim();
    
}

public String getDNS_ID(String IDTelemetria) //OK
{
    String strsql;
    String str_res;
        dbURL=url_access+dbName;
        strsql="select medidor.fk_id_dns_client from medidor where medidor.fk_id_telemetria='"+IDTelemetria+"';";
        //strsql="select medidor.fk_id_reporte from telemetria, medidor where telemetria.fk_id_modemsatelital='"+IDModem+"' and telemetria.id_telemetria = medidor.fk_id_telemetria;";
        try
        {
            str_res=getField(strsql).trim();
        }
        catch(Exception ex)
        {
            str_res=null;

        }
	return str_res;

}

public String getDNSName(String IDTelemetria) //OK
{
    String strsql;
    String str_res;
    String ID_DNSClient;

    ID_DNSClient=this.getDNS_ID(IDTelemetria);

    if(ID_DNSClient!=null)
    {
        dbURL=url_access+dbName;
        strsql="select dns_client.dns_ip from dns_client where dns_client.id_dns_client="+ID_DNSClient+";";
        try
        {
            str_res=getField(strsql).trim();
        }
        catch(Exception ex)
        {
            str_res=null;

        }
    }
    else
    {
        str_res=null;
    }
	return str_res;

}

public String getDNSIPNumber(String IDTelemetria) //OK
{
    String strsql;
    String str_res;
    String ID_DNSClient;

    ID_DNSClient=this.getDNS_ID(IDTelemetria);

    if(ID_DNSClient!=null)
    {
        dbURL=url_access+dbName;
        strsql="select dns_client.port_ip from dns_client where dns_client.id_dns_client="+ID_DNSClient+";";
        try
        {
            str_res=getField(strsql).trim();
        }
        catch(Exception ex)
        {
            str_res=null;

        }
    }
    else
    {
        str_res=null;
    }
	return str_res;

}

////////////////////////////////////////////////


}
