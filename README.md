Description en cours de réalisation

#Supervision SMS pour Alarme DSC (1616 et compatibles) à base Arduino+TC35

##Pourquoi une alarme DSC filaire ?
Les alamres DSC filaires sont sans doute les meilleures et les plus utilisées aussi bien chez les particuliers que dans les entreprises.

Les alarmes sans fils sont plus faciles à installer, mais aussi plus faciles à contrecarrer, à moins d'avoir un budget beaucoup plus important

##Montage
Ce montage permet :
- D'être notifié lors de l'armement, désarmement, déclenchement de l'alarme en étant notifié par SMS, d'une manière autonome
- De permettre via un Raspberry d'interagir avec l'arduino(Port USB)  pour :
  - Récupérer le statut de l'alarme
  - Envoyer / recevoir des SMS(Gateway)
  - Programmer le ou les numéros des destinataires des notifications dans l'EEPROM de l'arduino

Il est totalement indépendant de la supervision puisque l'ensemble utilise le bus des claviers déportés.

On ne peut pas utiliser ce montage pour activer ou désactiver l'alarme. Cela serait en plus une faille de sécurité.

Attention aussi à ne pas diffuser sans sécurité l'état de l'alarme si vous l'interconnecter avec votre solution de domotique

##Principaux Composants utilisés :
- Arduino Nano
- TC35 SaintSmart (http://www.sainsmart.com/siemens-tc35-sms-gsm-module-voice-adapter.html)
- 4 Résistance 10 kOhms (basé en partie sur les éléments de ce fil http://www.avrfreaks.net/forum/dsc-keybus-protocol)
- Un adaptateur 12V -> 5V 
 
Soit moins de 50 euros de composants...

ATTENTION :  ne pas connecter le TC35 sans que ce dernier ne soit alimenté par autre chose que l'arduino, au risque de détruire ce dernier.

##Schema de principe
![alt tag](https://github.com/diyfr/dsc_sms/blob/master/readme/shema.png)
##Le keybus
![alt tag](https://github.com/diyfr/dsc_sms/blob/master/readme/keybus.jpg)
##Vue intérieure
![alt tag](https://github.com/diyfr/dsc_sms/blob/master/readme/int-low.jpg)
##De l'exterieur
![alt tag](https://github.com/diyfr/dsc_sms/blob/master/readme/ext.jpg)
