package main

import (
	"bufio"
	"fmt"
	"net"
	"os"
	"strings"
)

func main() {
  listener, err := net.Listen("tcp", "127.0.0.1:8080")
  if err != nil {
    os.Exit(1);
  }
  defer listener.Close()
  fmt.Println("Server listening on", listener.Addr().String())

  for {
    conn, err := listener.Accept()
    if err != nil {
      continue
    }

    go handleClient(conn)
  }
}

// TODO: Parse msg and save states
func handleClient(conn net.Conn) {
  defer conn.Close()

  reader := bufio.NewReader(conn)
  for {
    message, err := reader.ReadString('\n')
    if err != nil {
      return
    }

    message = strings.TrimSpace(message)
    fmt.Println(message);

    conn.Write([]byte("received\n"))
  }
}
