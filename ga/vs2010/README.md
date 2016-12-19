# Gaming Anywhere (XS inc)

## 빌드 조건

1. DirectX SDK
2. Microsoft SDK (Windows 7 ↑)
3. VisualStudio 2010 ↑

2010 보다 높은 버전의 VS 를 사용하려면 2010을 함께 설치하여 개발은 상위 버전으로 하되, 빌드는 2010 버전으로 해야 합니다. (상위 버전 VS에서 프로젝트를 그대로 여신 후에 컴파일러 업데이트 문구가 나오면 취소를 누르시면 됩니다.)


## 설정 파일 관련 안내

[공식문서](http://gaminganywhere.org/doc/config.html)에 나와있는 설정은 제외합니다.

1. 서버

    - capture_mouse (enable, disable)
        > 화면에 마우스를 렌더링할 지의 여부를 결정하는 설정입니다.

2. 클라이언트

    - control-relative-mouse-mode (enable, disable)
        > 마우스가 화면에 렌더링 되는 방식을 결정하는 설정입니다. enable 로 설정하게 되면 해상도에서 상대적인 위치에서 마우스를 제어할 수 있게 됩니다. 반대로 disable 로 설정하면 절대적인 위치에서 마우스를 제어하게 됩니다.

    - fullscreen (enable, disable)
        > 클라이언트를 실행할 때 전체화면으로 실행할 지의 여부를 결정하는 설정입니다.