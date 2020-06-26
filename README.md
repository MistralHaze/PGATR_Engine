Construcción en sistema X-nix debian based

0)Actualizar
sudo apt-get update

1)Requisitos
sudo apt-get -y install build-essential
sudo apt-get -y install mesa-common-dev
sudo apt-get -y install libglm-dev
sudo apt-get install libglfw3-dev  -y 
sudo apt install -y libassimp-dev assimp-utils assimp-testmodels
 
#Instalación de las librerías estandar de desarrollo de vulkan (nvidia tested)
sudo apt install -y libvulkan1 libvulkan-dev
sudo apt install -y vulkan-tools vulkan-utils vulkan-validationlayers-dev

#Para poder tener vulkan en tarjetas intel (630 works) el driver esta basado en mesa!!!
#sudo apt install mesa-vulkan-drivers

sudo apt-get install cmake -y 

2)Construcción del proyecto
-> Ir al directorio build -> cd build
-> Configurar el proyecto utilizando cmake -> cmake .. 
	-> Añadir -DCMAKE_BUILD_TYPE=Release para compilar en release
	-> Revisar que no da errores
-> Construir el proyecto -> make -j

#Optativo
Instalación del LunarSDK -> Seguir instrucciones de la página oficial!
