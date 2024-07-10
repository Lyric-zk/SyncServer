#include <iostream>
#include <boost/asio.hpp>
#include <set>
#include <memory>
using boost::asio::ip::tcp;
const int max_length = 1024;
typedef std::shared_ptr<tcp::socket> socket_ptr;
std::set<std::shared_ptr<std::thread>> thread_set;//定义一个线程集合
using namespace std;
//这是第一次修改
void session(socket_ptr sock) {//在该函数中处理客户端发送过来的数据
	try{
		for (;;) {
			char data[max_length];
			memset(data, 0, max_length);
			boost::system::error_code error;
			//size_t length = boost::asio::read(sock, boost::asio::buffer(data, max_length),error);
			size_t length = sock->read_some(boost::asio::buffer(data, max_length), error);
			if (error == boost::asio::error::eof) {//eof是对端关闭的错误
				cout << "connection closed by peer" << endl;
				break;
			}
			else if (error) {
				throw boost::system::system_error(error);
			}
			if (!strcmp(data, "q")) {
				cout << sock->remote_endpoint().address().to_string()
					<<"[ "<< sock->remote_endpoint().port() << " ]" << "客户端主动断开连接..." << endl;
				return;
			}
			//客户端连接地址信息
			cout << "receive from 客户端[ " << sock->remote_endpoint().port();
			cout << " ] receive message is " << data << endl;
			snprintf(data, max_length, "%s--lyric", data);
			//回传给对方
			boost::asio::write(*sock, boost::asio::buffer(data, max_length));
		}
	}
	catch (const std::exception& e){
		std::cerr << "Exception in thread" << e.what() << "\n" << std::endl;
	}
}

void server(boost::asio::io_context& io_context, unsigned short port) {
	cout << "等待客户端的连接..." << endl;
	//acceptor是用于接收客户端连接的            本地地址    端口
	tcp::acceptor a(io_context, tcp::endpoint(tcp::v4(), port));
	for (;;) {
		socket_ptr socket(new tcp::socket(io_context));
		a.accept(*socket);//接收客户端连接
		//std::thread的构造函数接受一个可调用对象（如函数、lambda表达式、函数对象等）作为线程执行的入口点。
		cout << "客户端[ " << socket->remote_endpoint().port() << " ]已连接..." << endl;
		auto t = std::make_shared<std::thread>(session, socket);
		//就像是pthread_create函数 后两个参数一个是函数指针，一个是该函数所需要的参数
		thread_set.insert(t);
	}
}
int main()
{
	try{
		boost::asio::io_context ioc;
		server(ioc, 10086);
		for (auto& t : thread_set) {
			if (t->joinable()) {
				t->join();//join()函数用于等待线程的执行完成
			}
			//当调用join()时，程序会阻塞当前线程，直到被调用的线程执行完成
			//类似与pthread_join函数
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "主线程异常" << e.what() << endl;
	}
	return 0;
}
