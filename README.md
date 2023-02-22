# portfolio-mmoserver-cpp-boost-asio
C++ Boost asio 라이브러리 기반 MMO 서버 프레임워크


# 소개
C++ Boost asio 라이브러리 기반 MMO 서버 프레임워크입니다.


# 기능
:heavy_check_mark: 서버 아키텍처


:heavy_check_mark: Server Launcher


:heavy_check_mark: Network 서버


:heavy_check_mark: User 서버


:heavy_check_mark: Zone 서버


:heavy_check_mark: Chat 서버


# 서버 아키텍처
(도식화 그림)
- 서버는 다수의 세부 모듈로 구성되어 있습니다.
- 각 모듈은 기능적으로 분리되어 있습니다.
- 실제 서비스에서는 같은 모듈이라도 수용할 수 있는 접속 유저 수에 따라 여러 개로 분리될 수 있습니다.
- 각 모듈은 Connector 모듈을 통해 다른 모듈과 통신합니다. 
