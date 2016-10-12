# cv16-interface

Set of test images: 
http://www.mediafire.com/download/29wqk8fr8h5n8qf/CVInterfacePics.zip

Previous AUVSI data:
https://drive.google.com/drive/folders/0B7ik7oGEZVhMNkFvalBTeFZxcjA

# Instructions

1. Download & install git scm: https://git-scm.com/downloads
2. Download & install Qt 5.6 MSVC 64 bit: https://www.qt.io/download/
3. Download & install Visual Studio 2013 (last tested working version): https://www.visualstudio.com/vs/older-downloads/
4. Download the test images from links above
5. Go to the folder you want to clone this project into. Right click and select "Git Bash Here"
  * Enter in the git bash:
  ```
  git clone https://github.com/utat-uav/Software.git
  ```
6. Download these missing files (libraries) and put them in the Classification folders: https://www.mediafire.com/?kc2rmmr3hlcqh4p
7. Open the Software/Classification/SoftmaxRegression.sln file in Visual Studio 2013
  * Configure the project for 32 bit (x86) in release mode
  * Make sure the compiler kit is correct
  * Build and run
8. Open the Software/CVInterface.pro file with Qt
  * Configure the project to use the MSVC 64 bit kit
  * Build and run
