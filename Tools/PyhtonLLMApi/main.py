import socket
from ollama import chat, ChatResponse


# Define the tools
def add(a: int, b: int) -> str:
    return str(a + b)

def subtract(a: int, b: int) -> str:
    return str(a - b)

# Ollama tools dictionary
tools = {
    "add": add,
    "subtract": subtract
}

HOST = "127.0.0.1"
PORT = 65432

def query_ollama(prompt: str) -> str:
    """
    Query local Ollama model directly via Python API with tools enabled.
    """
    try:
        response: ChatResponse = chat(
            model="qwen3:8b",
            messages=[
                {"role": "user", "content": prompt}
            ]
        )
        return response.message.content.strip()
    except Exception as e:
        return f"[Error] {e}"

def main():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen()
        print("Python server ready (Qwen 3B via Ollama Python API). Waiting for C++ client...")
        conn, addr = s.accept()
        print(f"Connected by {addr}")

        with conn:
            while True:
                data = conn.recv(4096)
                if not data:
                    break
                prompt = data.decode("utf-8").strip()
                print(f"[C++] -> {prompt}")

                result = query_ollama(prompt)
                print(f"[Qwen 3B] -> {result[:80]}...")
                conn.sendall(result.encode("utf-8"))

if __name__ == "__main__":
    main()
