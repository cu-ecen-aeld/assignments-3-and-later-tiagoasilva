# !/bin/sh
# Start and stop the socket server

case $1 in
    start )
        echo "Starting aesdsocket"
        start-stop-deamon -S -n aesdsocket -n /usr/bin/aesdsocket
        ;;
    stop )
        echo "Stopping aesdsocket"
        start-stop-deamon -K -n aesdsocket 
        ;;
    * )
        echo "Usage: $0 {start|stop}"
        exit 1
        ;;
esac

exit 0