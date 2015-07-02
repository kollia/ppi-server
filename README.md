# ppi-server
The ppi-server project is an C++ linux based server for x86/x64 architecture, controlling external devices and displays data on an java thin-client using the SWT/JFace-Librarys. 

 	

The server controls a variety of sensors (like thermostat- light- humidity- sensors, IO switches, A/D and D/A converter …). Supporting COM-Interfaces directly, Maxim/Dallas semiconductors over the OWFS-project and the Vellemann k8055 board.

Sensors are configured using control lists within text files (beginning with measure.conf) and additionally in the same text files, control structures are provided for handling of the receipted values. Other applications can be invoked there too and Sensors can also be controlled extern by command line scripts.

The server provides a hierarchical folder structure of layout files (like HTML) under ppi-server/client in /etc/ , which can be assessed using by any clients supporting the ppi-server protocol (protocol description: './ppi-server -?'). The provided thin client is based on this protocol too.

	

        maintained: 
        by Alexander Kolli
        mailto:ppi@magnificat.at 

	

    The ppi-server is distributed in the hope that it will be useful.
    So please do not be chary if you discover an error and help me by reporting this bug.
    This project is still fairly new and it is assumed that I fix the problem soon as possible.
    The ppi-server team (only me this time) encourages contributions from the developer community.

    If you are interested in getting involved in making ppi-server even better,

    I welcome your participation (mailto:ppi-server-developer@googlegroups.com). 
