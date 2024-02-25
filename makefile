# 変数名
CXX=g++
CFLAGS=-std=c++14 -g -static
# makeの組み込みルールによって認識される変数. コンパイラに渡すコマンドオプション
# -std=c++11: C++11で書かれたソースコードということを伝える
# -g: デバッグ情報を出力
# -static: スタティックリンクする

SRCS=$(wildcard *.cpp)
# wildcard: makeが提供している関数. 引数にマッチするファイル名に展開される
# 今回は、main.cpp parse.cpp codegen.cppとなる

SRCS_FILE=$(notdir $(SRCS))

OBJS=$(SRCS_FILE:.cpp=.o)
# 変換の置換ルール
# SRCの.cppを.oに変換する

# 9ccファイルを作る
9cc: $(OBJS)
	$(CXX) -o 9cc $(OBJS) $(CFLAGS)
# g++ -o 9cc main.o parse.o codegen.o
# $(CC): gcc
# -o (名前): 実行ファイルに名前をつける
# main.o parse.o codegen.o はこれらのオブジェクトファイルから実行ファイルを作るという意味
# $(LDFLAGS): リンクオプション. 初期値は空. 動的ライブラリをリンクする場合は-lオプションなどを LDFLAGS = という感じで書ける

$(OBJS): 9cc.h
# すべての.oファイルが9cc.hに依存していることを表す
# つまり、9cc.hを変更した場合は全.oファイルが再コンパイルされる

test: 9cc
	./test.sh

clean:
	rm -f 9cc *.o *~ tmp*

.PHONY: test clean
# make foo と書くとfooというファイルを作成しようとし、存在する場合はソースコードが変更された時だけバイナリを再生成する
# .PHONY はダミーのターゲットを表す特別な名前
# make test, make clean はファイルを作成したいわけではないが、makeにはわからないので、
# .PHONYを指定し、指定されたターゲットのファイルが存在しているかどうかに関わらずルールのコマンドを実行するべきということをmakeに伝えている