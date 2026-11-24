FROM gcc:latest
WORKDIR /app
COPY tic.cpp .
RUN g++ -O2 tic.cpp -o tic_server
EXPOSE 8081
CMD ["./tic_server"]
