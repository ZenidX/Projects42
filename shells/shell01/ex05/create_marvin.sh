# El archivo a entregar se llama:  "\?$*'MaRViN'*$?\"
# Ese nombre es ilegal en NTFS (contiene " ? * \), asi que en Windows no puede
# existir: se crea con este script en Linux y se conserva en ex05.tar.
printf '42' > '"\?$*'\''MaRViN'\''*$?\"'
chmod 614 '"\?$*'\''MaRViN'\''*$?\"'
