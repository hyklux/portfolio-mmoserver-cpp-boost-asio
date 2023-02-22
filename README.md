# portfolio-mmoserver-cpp-boost-asio
C++ Boost asio 라이브러리 기반 MMO 서버 프레임워크


# 소개
C++ Boost asio 라이브러리 기반 MMO 서버 프레임워크입니다.


# 기능
:heavy_check_mark: 서버 아키텍처


:heavy_check_mark: Server Container


:heavy_check_mark: Network 모듈


:heavy_check_mark: User 모듈


:heavy_check_mark: Zone 모듈


:heavy_check_mark: Chat 모듈


# 서버 아키텍처
(도식화 그림)
- 서버는 다수의 세부 모듈로 구성되어 있습니다.
- 각 모듈은 기능적으로 분리되어 있습니다.
- 실제 서비스에서는 같은 모듈이라도 수용 가능한 접속 유저 수에 따라 여러 개로 분리될 수 있습니다.
- 각 모듈은 Connector 모듈을 통해 다른 모듈과 통신합니다.


# Server Container
- 서버가 시작되면 config.json 파일에 정의된 user, zone 등의 모듈을 로드합니다.
- 로드할 모듈은 해당 서버의 구성에 따라 달라질 수 있습니다.
- 예를 들어 어떤 서버에는 zone 모듈이 없다거나 어떤 서버에는 dbagent 모듈이 없을 수 있습니다.
- Server Container는 해당 서버에 업로드된 모든 모듈을 관리하는 컨테이너입니다.
- 프로그램 실행과 동시에 로드하는 모든 서버모듈을 참조하고 있습니다.


# Network 모듈


# User 모듈


# Zone 모듈


# Chat 모듈
