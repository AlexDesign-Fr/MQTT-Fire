pipeline {
    // Ce script utilise la méthode declarative pour les pipelines
    // @see https://www.lambdatest.com/blog/jenkins-declarative-pipeline-examples/
    // ---------------------------------------------------------------------------
    agent none
    environment {
        PROJECT_NAME = "MQTT-fire"
    }
	stages {
		stage('Lancement en parallèle de stages') {
              parallel {
				stage('Récupération de la version du code (git tag)') {
					agent any
					steps {
						script {
							echo '*****************************************************************************'
							echo 'Récupération des sources pour avoir le tag de la version'
							checkout scm
							latestTag = sh(returnStdout:  true, script: "git describe --tags --abbrev=0").trim()
							env.BUILD_VERSION = latestTag
							echo '*****************************************************************************'
						}
					}
				}    
				stage('Download sources sur VM compilation') {
					agent { label 'vagrant'}
					steps {
						script{
							dir("${PROJECT_NAME}"){
								checkout scm
							}
						}
					}
				}
				stage('Download librairies pour compilation Arduino sur VM compilation') {
					agent { label 'vagrant'}
					steps {
						script{
							sh 'arduino-cli lib install "Adafruit NeoPixel"'
							sh 'arduino-cli lib install "PubSubClient"'
							sh 'arduino-cli lib install "WiFiManager"'
						}
					}
				}
				stage("Import des scripts de génération pour le site web (VM Alexdesign)"){
					agent { label 'alexdesign'}
					steps {
						script {
							deleteDir()
							dir( "jenkinsWebProject" ) {
								sh 'git clone https://git.alex-design.fr/Alex/jenkinsWebProject.git ./'
								sh 'chown -R alexandre.alexandre *'
								sh 'mkdir -p /data/www/html/update/$PROJECT_NAME'
								sh 'sudo chown alexandre.alexandre -R /data/www/html/update/$PROJECT_NAME'

								// test si le répertoire destination existe déjà
								echo '*****************************************************************************'
								def fileName = "/var/www/html/update/$PROJECT_NAME/themes"
								if ( !fileExists(fileName) ) {
									echo "Déplacement des fichiers générant la page web"
									sh 'mv -fu * /data/www/html/update/$PROJECT_NAME/'
								} else {
									echo "Les fichiers générant la page web existent déjà, on ne trannsfère que le générateur, pas le theme (il peut avoir été customisé)."
									sh 'mv -fu *.php /data/www/html/update/$PROJECT_NAME/'
								}
								echo '*****************************************************************************'
							}
						}
					}
				}
			}
		}
        stage("Compilation (VM compilation)"){
            agent { label 'vagrant'}
            steps {
                script{
                    dir("${PROJECT_NAME}"){
                        sh 'arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 ${PROJECT_NAME}.ino --output-dir /partage'
                    }
                }
            }
        }
        stage("Déplacement du binaire de VM compilation vers host"){
            agent { label 'host'}
            steps {
                script {
                    sh 'mv /var/lib/vagrant/VMCOmpileArduino/partage/${PROJECT_NAME}.ino.bin /partage'
                    sh 'chown alexandre.alexandre /partage/${PROJECT_NAME}.ino.bin'
                }
            }
        }
        stage("Transfère du README dans le site web (VM Alexdesign)"){
            agent { label 'alexdesign'}
            steps {
                script{
                    sh 'mkdir -p /data/www/html/update/$PROJECT_NAME/bin'
                    sh 'mkdir -p /data/www/html/update/$PROJECT_NAME/inc'
                    checkout scm
                    sh 'mv -fu README.md /data/www/html/update/$PROJECT_NAME/inc/'

                    def fileName = "./$PROJECT_NAME/doc"
                    if ( fileExists(fileName) ) {
                        sh 'rm -rf /data/www/html/update/$PROJECT_NAME/doc'
                        sh 'mv -fu ./doc /data/www/html/update/$PROJECT_NAME/'
                    }
                }
            }
        }
        stage("Déplacement du binaire vers le site web (VM Alexdesign)"){
            agent { label 'alexdesign'}
            steps {
                script{
                    dir("/data/www/html/update/$PROJECT_NAME/bin"){
                        sh "cp -f /partage/${PROJECT_NAME}.ino.bin ./${PROJECT_NAME}__${env.BUILD_VERSION}.bin"
                    }
                }
            }
        }
        stage("Extraction des infos de README vers la page HTML"){
            agent { label 'alexdesign'}
            steps {
                script{
                    dir("/data/www/html/update/$PROJECT_NAME/inc"){
                        sh 'pandoc -s README.md  --template ../templateDATA.php -o data.php'
                    }
                }
            }
        }
        stage("Ajustement des droits pour apache sur le le site web (VM Alexdesign)"){
            agent { label 'alexdesign'}
            steps {
                script{
                    sh 'sudo chown apache.apache -R /data/www/html/update/$PROJECT_NAME'
                }
            }
        }
    }
}
