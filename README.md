<h2>Pintos Project</h2>
2022-Fall Sogang Univ. 운영체제(CSE4070) 
<h2>Project1. User Program(1)</h2>

User program이 Pintos 위에서 정상적으로 구동되도록 하기 위한 기능을 구현

-Argument Passing

+ 입력으로 들어온 command line에서 argument들을 parsing하고 80x86 calling convention에 맞춰 stack에 push

-User Memory Access

+ User program이 Kenrel adreess에 접근하거나 null pointer에 접근할 경우 invalid pointer로 간주하여 pass

-System Calls

+ System call halt, exit, exec,  wait, read, write의 기능 구현 

+ Read와 write은 각각 standard input, standard output에 대해서만 구현

+ additional system call로 fibonacci와 max_of_four_int를 구현


<h2>Project2. User Program(2)</h2>

file system과 관련된 system call들을 구현하여 pintos에서 user program을 완성

-File Descriptor

+ thread마다 개별적인 file descriptor table을 생성

+ 파일을 열 때마다 서로 다른 file descriptor 값을 부여하여 file descriptor table을 통한 파일 관리 구현

-System Calls

+ system call create, remove, open, close, filesize, read, write, seek, tell 기능 구현

+ open 과정에서 executable file에 대한 쓰기 작업이 수행되지 않도록 보호

+ read STDIN, write STDOUT과 더불어 File system과 관련된 모든 system call 기능 구현

-Synchronization in Filesystem

+ file system에서의 synchronization 기능 구현

+ 코드의 critical section을 보호하고 각 프로세스가 shared data에 독점적으로 접근하도록 함


<h2>Project3. Threads</h2>

-Alarm clock

+ 기존 Pintos에서 Busy waiting으로 구현된 Alarm 기능을 효율적으로 개선

+ 일어날 시간이 되지 않은 스레드를 Block 시키는 방법으로 Sleep과 Wake up 기능을 구현 

-Priority Scheduling

+ Pintos의 Round-robin scheduler 보완

+ 새로운 스레드가 ready list에 삽입될 때 priority를 고려하도록 하여 스레드가 각자의 priority에 맞는 순서대로 삽입되고 수행될 수 있도록 함

-Advanced Scheduler

+ BSD Scheduler 구현

+ recent_cpu와 nice의 값을 이용하여 스레드의 priority를 계산하므로 기존 방식보다 효율적인 threads scheduling 구현

