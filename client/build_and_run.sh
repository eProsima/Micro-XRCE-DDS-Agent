if [ "$1" = "proto_client" ]; then
    make prototype
    if [ $? -eq 0 ]; then
        build/prototype
    fi        
elif [ "$1" = "proto_agent" ]; then
    make prototype_agent_test
    if [ $? -eq 0 ]; then
        build/prototype_agent_test
    fi
elif [ "$1" = "message" ]; then
    make message_test
    if [ $? -eq 0 ]; then
        build/message_test
    fi
elif [ "$1" = "mini_cdr" ]; then
    make mini_cdr_test
    if [ $? -eq 0 ]; then
        build/mini_cdr_test
    fi
elif [ "$1" = "mini_cdr_p" ]; then
    make mini_micro_cdr_performance_comparative_test
    if [ $? -eq 0 ]; then
        build/mini_micro_cdr_performance_comparative_test
    fi
else
    echo "ERROR => Enter a valid build."
fi
