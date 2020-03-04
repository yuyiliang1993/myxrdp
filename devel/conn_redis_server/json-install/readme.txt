oncpp源码 
http://sourceforge.net/projects/jsoncpp/files/
解压jsoncpp到/opt/json下 
tar -zvxf jsoncpp-src-0.5.0.tar.gz -C /opt/json 
3.下载完后阅读README，发现需要用Scons来构建，关于Scons的介绍参考:http://os.51cto.com/art/201104/257443.htm，那么就下载Scons吧
下载Scons 
http://sourceforge.net/projects/scons/files/scons/2.1.0/scons-2.1.0.tar.gz/download
解压Scons到/opt/json下 
tar -zvxf scons-2.1.0.tar.gz -C /opt/json
进入scons-2.1.0目录下，执行以下命令 
sudo python setup.py install
进入jsoncpp-src-0.5.0 目录下，执行 
sudo scons platform=linux-gcc
将/jsoncpp-src-0.5.0/include/目录下的json文件夹拷贝到/usr/include/
将jsoncpp-src-0.5.0/libs/linux-gcc-4.9.1/目录下的libjson_linux-gcc-4.9.1_libmt.a 拷贝到/usr/local/lib/下，并为了方便使用，将其重命名为libjson.a
