main:
	g++ -o mygit main.cpp init.cpp hashobject.cpp catfile.cpp add.cpp writetree.cpp lstree.cpp commit.cpp log.cpp checkout.cpp -lzstd -lssl -lcrypto