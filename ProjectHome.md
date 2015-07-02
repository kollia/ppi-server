The **ppi-server** project is an C++ linux based server for x86/x64 architecture, controlling external devices and displays data on an java thin-client using the SWT/JFace-Librarys.
<table>
<blockquote><tr>
<blockquote><td width='100'>
</td>
<td>
<blockquote><img src='http://lh3.ggpht.com/_Gc4rkozOlWc/Sj-9EvcVqdI/AAAAAAAAAJg/JoidCax3RFc/ppi-server300x105.png' />
</blockquote></td>
</blockquote></tr>
</table>
The server controls a variety of sensors (like thermostat- light- humidity- sensors, IO switches, A/D and D/A converter …).<br>
Supporting COM-Interfaces directly, <a href='http://www.maxim-ic.com'>Maxim/Dallas</a> semiconductors over the <a href='http://www.owfs.org'>OWFS</a>-project and the <a href='http://www.velleman.be/'>Vellemann</a> <a href='http://libk8055.sourceforge.net/'>k8055</a> board.<br />
Sensors are configured using control lists within text files (beginning with measure.conf)<br>
and additionally in the same text files, control structures are provided for handling of the receipted values.<br>
Other applications can be invoked there too and Sensors can also be controlled extern by command line scripts.</blockquote>

The server provides a hierarchical folder structure of layout files (like HTML) under ppi-server/client in /etc/ , which can be assessed using by any clients supporting the ppi-server protocol (protocol description: './ppi-server -?'). The provided thin client is based on this protocol too.
<table>
<blockquote><tr>
<blockquote><td>
</td>
<td>
<blockquote><table>
<blockquote><tr>
<blockquote><td valign='top'>
<blockquote><b>maintained:</b>
</blockquote></td>
<td>
<blockquote>by Alexander Kolli<br>
<br />
<a href='mailto:ppi@magnificat.at'>mailto:ppi@magnificat.at</a>
</blockquote></td>
</blockquote></tr>
</blockquote></table>
</blockquote></td>
</blockquote></tr>
<tr>
<blockquote><td>
</td>
<td>
<blockquote>The ppi-server is distributed in the hope that it will be useful.<br />
So please do not be chary if you discover an error and help me by<br>
<a href='http://code.google.com/p/ppi-server/issues/list'>reporting</a> this bug.<br />
This project is still fairly new<br>
and it is assumed that I fix the problem soon as possible.<br />
The ppi-server team (only me this time) encourages contributions from the developer community.<br />
If you are interested in getting involved in making ppi-server even better, <br />
I welcome your participation (<a href='mailto:ppi-server-developer@googlegroups.com'>mailto:ppi-server-developer@googlegroups.com</a>).<br>
</blockquote></td>
</blockquote></tr>
</table>