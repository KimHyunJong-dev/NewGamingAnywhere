gaminganywhere
==============

GamingAnywhere: An Open Cloud Gaming System

* Overview
GamingAnywhere is an open-source clouding gaming platform. In addition to its
openness, we design GamingAnywhere for high extensibility, portability, and
reconfigurability. GamingAnywhere currently supports Windows and Linux, and
can be ported to other OS's including OS X and Android.


* background 실행 (image.png 참조)

1. 해당 프로젝트 오른쪽 클릭 -> 속성 진입합니다.

2. 링커 -> 시스템 탭으로 접속합니다.

3. 하위시스템 콘솔 -> 창 으로 변경합니다.

4. 해당 프로젝트 main 함수를 WinMain 함수로 변경합니다.

5. main 함수의 argc, argv 부분을 __argc, __argv 로 변경합니다.


* Memo

프로젝트 빌드 설정  ( 참조: http://gaminganywhere.org/doc/quick_start.html)
작업 환경은 Visual Studio 2015 입니다.

* 의존성 설치 

- DirectX - https://www.microsoft.com/en-us/download/details.aspx?id=6812 

- MicrosoftSDK - https://www.microsoft.com/en-us/download/details.aspx?id=8279
 
1. VisualStudio 2015 을 통해 프로젝트를 엽니다.
2. 솔루션 전체 빌드를 통해 문제가 있는 프로젝트를 선별합니다.
3. 발견된 프로젝트 오른쪽 클릭 -> 속성에 들어갑니다.
4. C/C++ 메뉴를 클릭하신 후에 추가 포함 디렉토리 (DirectX)에서 경로를 설정합니다.
5. 링커 메뉴를 클릭하신 후에 추가 라이브러리 디렉토리 (DirectX)에서 경로를 설정합니다.
6. 솔루션 전체 빌드를 통해 빌드를 완료합니다.

* Gaming Anywhere 수정본 

Gaming Anywhere 에서 사용할 수 있는 (파악한) config 설정은 아래와 같습니다.

[server.desktop.conf 설정]

capture_cursor (enable, disable) 항목

마우스를 캡쳐할 지 말 지를 결정하는 설정입니다.

[client.abs.conf 또는 client.rel.conf 설정]

control-relative-mosue-mode (enable, disable) 

마우스를 어떤 방식으로 제어할 지 결정하는 설정입니다.

enable 로 설정하시게 되면, relative 한 위치에서 마우스를 제어하실 수 있습니다. 
enable 모드를 설정하시면 서버에서 capture_cursor 설정을 enable 로 주셔야 합니다.

fullscreen 항목추가: 클라이언트를 전체화면으로 실행할 지 결정하는 설정입니다.

* Documents

Official web site: http://gaminganywhere.org/

Quick start guide: http://gaminganywhere.org/doc/quick_start.html

Configuration file guide: http://gaminganywhere.org/doc/config.html

FAQ: http://gaminganywhere.org/faq.html

