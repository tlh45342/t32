FROM python:3.12-slim

WORKDIR /app

COPY t32core.py vconsole_client.py t32-node.py t32-node-dbg.py ./
COPY samples ./samples

RUN chmod +x /app/t32-node.py /app/t32-node-dbg.py

ENTRYPOINT ["python", "/app/t32-node.py"]
CMD ["/app/samples/hello.t32"]
