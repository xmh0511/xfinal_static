#include <iostream>
#include "http_server.hpp"
#include "client.hpp"
using namespace xfinal;

int main(std::size_t argus_size, char const* arugs[])
{
	std::cout << argus_size << std::endl;
	std::string port = "8080";
	std::string root_path = "D:\\";
	if (argus_size >= 3) {
		port = arugs[1];
		root_path = arugs[2];
	}
	http_server server(4);
	server.set_chunked_size(3 * 1024 * 1024);
	server.listen("0.0.0.0", port);
	server.router<GET>("/nginx/*", [&root_path](request& req,response& res) {
		auto url = req.url();
		std::string path;
		if (url == nonstd::string_view("/nginx")) 
		{
			path = "";
		}
		else {
			path = view2str(url.substr(nonstd::string_view("/nginx/").size(), url.size()));
		}
		auto fpath = fs::path(root_path + view2str(path));
		if (fs::is_directory(fpath)) {
			json root;
			root["list"]["directors"] = json::array();
			root["list"]["files"] = json::array();
			for (auto& iter : fs::directory_iterator(fpath)) {
				json it;
				bool is_direcotry = false;
				if (fs::is_directory(iter)) {
					it["is_directory"] = true;
					is_direcotry = true;
				}
				else {
					it["is_directory"] = false;
				}
				it["name"] = iter.path().filename();
				it["path"] = fs::relative(iter.path(), root_path);
				if (is_direcotry) {
					root["list"]["directors"].push_back(it);
				}
				else {
					root["list"]["files"].push_back(it);
				}
			}
			res.write_view("./tmpl/index.html", root,true,http_status::ok);
		}
		else {
			auto path = utf8_to_gbk(url_decode(fpath.string()));
			res.write_file(path, true); 
		}
	});
	server.run();
	return 0;
}