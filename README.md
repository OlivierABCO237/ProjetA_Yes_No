# ProjetA_Yes_No
Projet Entrainement d'un modèle avec TensorFlow et reconnaissance de gestes
## Etape 1 : ** Récupération des données ** 
- Hardware : _Carte Arduino Nono 33 BLE_
- Software: Le code _Generate_data_to_train.ino_ pour créer les données et les envoyer sur un port série
            Le code _serial_data_to_train.py_ pour recupérer les données via la liaison série, et les mettre dans deux fichiers: _No.csv_ et _Yes.csv_

  ## Etape 2: ** Entrainement des données **
  - software: Google Colab, et TensorFlow pour entrainer le modèle grace à un réseau de neuronnes
              Génération d'un fichier _model.h_ qui est le résultat de l'entrainement sous la forme d'une matrice de nombres
 
  ## Etape 3: ** Integration avec Arduino **
  - Hardware : _Carte Arduino Nono 33 BLE_
  - Software: Code _Classify_Imu.ino_, qui permet de tester notre modèle, et de voir s'il reconnais bien les gestes effectués.
              Code _Model.h_ qui inclus dans le Classify_Imu.ino, et contient toutes nos données cibles. Les données recupérées l'ors des tests suivants sont comparés à celles ci pour reconnaitre les gestes.

    ## Etape 4: **Quiz**
    Un quiz de 3 questions est posé à l'utilisateur. Après chaque question, l'algorithme de recupération de données et de tests est initialisé.
    APrès une réponse de l'utilisateur, ** une comparaison est faite entre le pourcentage de "yes" et de "no", pour recupérer son choix**
    Son choix est utilisé pour initier une réponse précise et est **gardé en mémoire"".
    Pour chaque question, **de nouvelles variables sont utilisées**.
    Le processus est repété trois fois.
    
