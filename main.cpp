#include <iostream>
#include "xfinal.hpp"
using namespace xfinal;

int main(std::size_t argus_size, char const* arugs[])
{
	std::string port = "8080";
	std::string root_path = "D:\\";
	if (argus_size >= 3) {
		port = arugs[1];
		root_path = arugs[2];
	}
	std::cout << "set http port: " << port << std::endl;
	std::cout << "set root path: " << root_path << std::endl;
	http_server server(4);
	server.set_chunked_size(3 * 1024 * 1024);
	server.listen("0.0.0.0", port);
	server.add_view_method("urlencode", 1, [](inja::Arguments& args)->json {
		auto path = args.at(0)->get<std::string>();
		return url_encode(path);
		});
	server.router<GET>("/nginx/*", [&root_path](request& req, response& res) {
		auto url = req.url();
		std::string path;
		if (url == nonstd::string_view("/nginx"))
		{
			path = "";
		}
		else {
			path = view2str(url.substr(nonstd::string_view("/nginx/").size(), url.size()));
		}
		auto fpath = fs::path(root_path + url_decode(view2str(path)));
		if (fs::is_directory(fpath)) {
			json root;
			root["list"]["directors"] = json::array();
			root["list"]["files"] = json::array();
			try {
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
			}
			catch (fs::filesystem_error const& ec) {
				std::cout << "error: " << xfinal::utf8_to_gbk(ec.what()) << std::endl;
			}
			res.write_view("./tmpl/index.html", root, true, http_status::ok);
		}
		else {
			res.write_file(utf8_to_gbk(fpath.string()), true);
		}
	});
	server.run();
	return 0;
}