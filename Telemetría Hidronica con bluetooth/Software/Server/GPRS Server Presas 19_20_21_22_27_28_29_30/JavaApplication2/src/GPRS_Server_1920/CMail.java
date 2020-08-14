/*
 * CMail.java
 *
 * Created on August 6, 2007, 12:02 PM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */
package GPRS_Server_1920;
/**
 *
 * @author Bolagnos
 */

 import java.util.*;
 import javax.mail.*; 
 import javax.mail.internet.*; 
 import javax.activation.*; 
 import java.io.*; 
 import java.util.Properties; 

 public class CMail 
 { 
  
  
     public void sendMailMessage(String to, String to_CC,final String from, final String psw, String host, String msgSubject, String msgText) //throws MessagingException, AddressException
     { 
         // Setup mail server 
         Properties props = System.getProperties(); 
         props.put("mail.smtp.host", host);
         props.put("mail.smtp.socketFactory.port", "465");
         props.put("mail.smtp.socketFactory.class","javax.net.ssl.SSLSocketFactory");
    	 props.put("mail.smtp.auth", "true");
         props.put("mail.smtp.port", "465");

          
         // Get a mail session 
         Session session = Session.getDefaultInstance(props,new javax.mail.Authenticator() {
            protected PasswordAuthentication getPasswordAuthentication()
            { return new PasswordAuthentication(from,psw);	}

         });
         session.setDebug(false);
         try
         {
         // Define a new mail message 
         Message msg = new MimeMessage(session); 
         msg.setFrom(new InternetAddress(from)); 
         //msg.addRecipient(Message.RecipientType.TO, new InternetAddress(to));
         msg.setRecipients(Message.RecipientType.TO, InternetAddress.parse(to));
         if(!to_CC.isEmpty())
         {
            msg.setRecipients(Message.RecipientType.CC, InternetAddress.parse(to_CC));
         }
         msg.setSubject(msgSubject); 
         msg.setSentDate(new Date());
         msg.setText(msgText);


         //System.out.println(msg.getContentType());
         //System.out.println(msg.getHeader("Content-Transfer-Encoding"));
          
         // Create a message part to represent the body text 
         //BodyPart messageBodyPart = new MimeBodyPart();
         //messageBodyPart.setText(msgText);
          
         //use a MimeMultipart as we need to handle the file attachments 
         //Multipart multipart = new MimeMultipart();
          
         //add the message body to the mime message 
         //multipart.addBodyPart(messageBodyPart);
          
         // add any file attachments to the message 
         //addAtachments(attachments, multipart);
          
         // Put all message parts in the message 
         //msg.setContent(multipart);
          
         // Send the message
         //Transport transport=session.getTransport("smtp");
         //transport.connect(host, 587, from, "04fbg40");
         Transport.send(msg);
         }
         catch (MessagingException mex) {
	    System.out.println("\n--Exception handling in msgsendsample.java");

	    mex.printStackTrace();
	    System.out.println();
	    Exception ex = mex;
	    do {
		if (ex instanceof SendFailedException) {
		    SendFailedException sfex = (SendFailedException)ex;
		    Address[] invalid = sfex.getInvalidAddresses();
		    if (invalid != null) {
			System.out.println("    ** Invalid Addresses");
			if (invalid != null) {
			    for (int i = 0; i < invalid.length; i++) 
				System.out.println("         " + invalid[i]);
			}
		    }
		    Address[] validUnsent = sfex.getValidUnsentAddresses();
		    if (validUnsent != null) {
			System.out.println("    ** ValidUnsent Addresses");
			if (validUnsent != null) {
			    for (int i = 0; i < validUnsent.length; i++) 
				System.out.println("         "+validUnsent[i]);
			}
		    }
		    Address[] validSent = sfex.getValidSentAddresses();
		    if (validSent != null) {
			System.out.println("    ** ValidSent Addresses");
			if (validSent != null) {
			    for (int i = 0; i < validSent.length; i++) 
				System.out.println("         "+validSent[i]);
			}
		    }
		}
		System.out.println();
		if (ex instanceof MessagingException)
		    ex = ((MessagingException)ex).getNextException();
		else
		    ex = null;
	    } while (ex != null);
	}
  
  
     } 
  
     /*protected void addAtachments(String[] attachments, Multipart multipart) 
                     throws MessagingException, AddressException 
     { 
         for(int i = 0; i<= attachments.length -1; i++) 
         { 
             String filename = attachments[i]; 
             MimeBodyPart attachmentBodyPart = new MimeBodyPart(); 
              
             //use a JAF FileDataSource as it does MIME type detection 
             DataSource source = new FileDataSource(filename); 
             attachmentBodyPart.setDataHandler(new DataHandler(source)); 
              
             //assume that the filename you want to send is the same as the 
             //actual file name - could alter this to remove the file path 
             attachmentBodyPart.setFileName(filename); 
              
             //add the attachment 
             multipart.addBodyPart(attachmentBodyPart); 
         } 
     }*/ 
  
 }