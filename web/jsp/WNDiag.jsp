<%@ page import="com.articulate.sigma.wordNet.WNdiagnostics" %>
<%@ include file="Prelude.jsp" %>
<html>                                             
<head><title> Knowledge base Browser </title></head>
<body BGCOLOR="#FFFFFF">

<%
/** This code is copyright Articulate Software (c) 2003.  Some portions
copyright Teknowledge (c) 2003 and reused under the terms of the GNU license.
This software is released under the GNU Public License <http://www.gnu.org/copyleft/gpl.html>.
Users of this code also consent, by use of this code, to credit Articulate Software
and Teknowledge in any writings, briefings, publications, presentations, or 
other representations of any software which incorporates, builds on, or uses this 
code.  Please cite the following article in any publication with references:

Pease, A., (2003). The Sigma Ontology Development Environment, 
in Working Notes of the IJCAI-2003 Workshop on Ontology and Distributed Systems,
August 9, Acapulco, Mexico.  See also http://sigmakee.sourceforge.net
*/
  StringBuffer show = new StringBuffer();       // Variable to contain the HTML page generated.
  String kbHref = null;
  String htmlDivider = "<table ALIGN='LEFT' WIDTH='50%'><tr><TD BGCOLOR='#A8BACF'><IMG SRC='pixmaps/1pixel.gif' width=1 height=1 border=0></TD></tr></table><BR><BR>\n";
  String formattedFormula = null;
  Map theMap = null;
  kbHref = "http://" + hostname + ":" + port + "/sigma/WordNet.jsp?lang=" + language + "&kb=" + kbName;
  try {
       WordNet.initOnce();
  }
  catch (Exception e) {
      System.out.println("Error in WNDiag.jsp:" + e.getMessage());
      e.printStackTrace();
      out.print("<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=KBs.jsp\">");
  } 
%>

<FORM action="WNDiag.jsp">
    <table width="95%" cellspacing="0" cellpadding="0">
        <tr>
            <td align="left" valign="top"><img src="pixmaps/sigmaSymbol-gray.gif"></td>
            <td>&nbsp;</td>
            <td align="left" valign="top"><img src="pixmaps/logoText-gray.gif"><br>
                <B>Knowledge Base Diagnostics</B></td>
            <td valign="bottom"></td>
            <td><b>[ <a href="KBs.jsp">Home</b></a>&nbsp;|&nbsp;
                <A href="AskTell.jsp?kb=<%=kbName %>&lang=<%=language %>"><b>Ask/Tell</b></A>&nbsp;|&nbsp;
                <a href="Properties.jsp"><b>Prefs</b></a>&nbsp;
                <B>]</B> <br>
                <img src="pixmaps/1pixel.gif" HEIGHT="3"><br>
                <b>KB:&nbsp;
<%
out.println(HTMLformatter.createKBMenu(kbName));  
%>              
                </b>
                <b>Language:&nbsp;<%= HTMLformatter.createMenu("lang",language,kb.availableLanguages()) %></b>
            </td>
        </tr>
    </table>
    <br>
</form>
<br>

<%
  out.println(WordNetUtilities.printStatistics());

  ArrayList synsetsWithoutTerms = WNdiagnostics.synsetsWithoutTerms();
    ArrayList nonRelationTermsWithoutSynsets = WNdiagnostics.nonRelationTermsWithoutSynsets();
  ArrayList synsetsWithoutFoundTerms = WNdiagnostics.synsetsWithoutFoundTerms(kb);
  ArrayList nonMatchingTaxonomy = WNdiagnostics.nonMatchingTaxonomy(kbName,language);

boolean isError = !(synsetsWithoutTerms.isEmpty() &&
                    synsetsWithoutFoundTerms.isEmpty() &&
                    nonMatchingTaxonomy.isEmpty());

if (!isError) 
    out.println("<br><b>&nbsp;No errors found</b>");
else {

    if (!nonRelationTermsWithoutSynsets.isEmpty()) {
        show.setLength(0);
        out.println("<table ALIGN=\"LEFT\" WIDTH=\"50%\"><tr><td BGCOLOR=\"#A8BACF\"><img src=\"pixmaps/1pixel.gif\" width=\"1\" height=\"1\" border=\"0\"></td></tr></table><p>");
        show.append(HTMLformatter.synsetList(nonRelationTermsWithoutSynsets,kbHref));
        out.println("<br><b>&nbsp;Error: nonRelationTermsWithoutSynsets</b>");
        out.println(show.toString() + "<br>");
    }

    if (!synsetsWithoutTerms.isEmpty()) {
        show.setLength(0);
        out.println("<table ALIGN=\"LEFT\" WIDTH=\"50%\"><tr><td BGCOLOR=\"#A8BACF\"><img src=\"pixmaps/1pixel.gif\" width=\"1\" height=\"1\" border=\"0\"></td></tr></table><p>");
        show.append(HTMLformatter.synsetList(synsetsWithoutTerms,kbHref));
        out.println("<br><b>&nbsp;Error: synsetsWithoutTerms</b>");
        out.println(show.toString() + "<br>");
    }

    if (!synsetsWithoutFoundTerms.isEmpty()) {
        show.setLength(0);
        out.println("<table ALIGN=\"LEFT\" WIDTH=\"50%\"><tr><td BGCOLOR=\"#A8BACF\"><img src=\"pixmaps/1pixel.gif\" width=\"1\" height=\"1\" border=\"0\"></td></tr></table><p>");
        show.append(HTMLformatter.synsetList(synsetsWithoutFoundTerms,kbHref));
        out.println("<br><b>&nbsp;Error: synsetsWithoutFoundTerms</b>");
        out.println(show.toString() + "<br>");
    }

    if (!nonMatchingTaxonomy.isEmpty()) {
        show.setLength(0);
        out.println("<table ALIGN=\"LEFT\" WIDTH=\"50%\"><tr><td BGCOLOR=\"#A8BACF\"><img src=\"pixmaps/1pixel.gif\" width=\"1\" height=\"1\" border=\"0\"></td></tr></table><p>");
        for (int i = 0; i < nonMatchingTaxonomy.size(); i++) {
            show.append((String) nonMatchingTaxonomy.get(i));
        }
        out.println("<br><b>&nbsp;Error: nonMatchingTaxonomy</b>");
        out.println(show.toString() + "<br>");
    }
    out.println("<table ALIGN=\"LEFT\" WIDTH=\"50%\"><tr><td BGCOLOR=\"#A8BACF\"><img src=\"pixmaps/1pixel.gif\" width=\"1\" height=\"1\" border=\"0\"></td></tr></table><p>");
}

%>
<p>

<%@ include file="Postlude.jsp" %>

</body>
</html>
