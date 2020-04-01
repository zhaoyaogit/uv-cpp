﻿/*
   Copyright © 2017-2020, orcaer@yeah.net  All rights reserved.

   Author: orcaer@yeah.net

   Last modified: 2020-3-30

   Description: https://github.com/wlgq2/uv-cpp
*/

#include "../include/http/Response.h"
#include "../include/LogWriter.h"

using namespace uv;
using namespace uv::http;

Response::Response()
    :Response(HttpVersion::Http1_1, StatusCode::OK)
{
}

Response::Response(HttpVersion version, StatusCode code)
    :version_(version),
    statusCode_(code)
{
}

void Response::setVersion(HttpVersion version)
{
    version_ = version;
}

void Response::setStatusCode(StatusCode code)
{
    statusCode_ = code;
}

HttpVersion Response::getVersion()
{
    return version_;
}

Response::StatusCode Response::getStatusCode()
{
    return statusCode_;
}

void Response::appendHead(std::string& key, std::string& value)
{
    heads_[key] = value;
}

void Response::appendHead(std::string&& key, std::string&& value)
{
    heads_[key] = value;
}

int uv::http::Response::appendHead(std::string& str)
{
    auto pos = str.find(": ");
    if (pos == str.npos)
    {
        return -1;
    }
    std::string key = std::string(str, 0, pos);
    std::string value = std::string(str, pos + 2);
    appendHead(key, value);
    return 0;
}

std::string Response::getHead(std::string& key)
{
    auto it = heads_.find(key);
    if (it == heads_.end())
    {
        return "";
    }
    return it->second;
}

void Response::swapContent(std::string& body)
{
    content_.swap(body);
}

void Response::swapContent(std::string&& body)
{
    content_.swap(body);
}

std::string& Response::getContent()
{
    return content_;
}

int Response::pack(std::string& data)
{
    return 0;
}

int Response::unpack(std::string& data)
{
    std::vector<std::string> headList;
    auto pos = SplitHttpOfCRLF(data, headList);
    if (pos == -1)
    {
        //解析失败
        return -1;
    }
    //解析状态行
    if (parseStatus(headList[0]) != 0)
    {
        return -1;
    }
    //解析消息头
    for (auto i = 1; i < headList.size(); i++)
    {
        if (appendHead(headList[i]) != 0)
        {
            return -1;
        }
    }
    //body数据
    content_ = std::string(data, pos + 4);
    return 0;
}

int Response::unpackAndCompleted(std::string& data)
{
    int rst = unpack(data);
    if (rst == 0)
    {
        auto it = heads_.find("Content-Length");
        if (it == heads_.end())
        {
            it = heads_.find("content-length");
        }
        if (it != heads_.end())
        {
            try
            {
                int size = std::stoi(it->second);
                return size != content_.size();
            }
            catch (...)
            {

            }
        }
    }
    return -1;
}

int Response::parseStatus(std::string& str)
{
    std::vector<std::string> out;
    auto pos = SplitStrOfSpace(str, out);
    if (out.size() != 3)
    {
        //解析失败
        return -1;
    }
    version_ = GetHttpVersion(out[0]);

    try
    {
        statusCode_ = (StatusCode)std::stoi(out[1]);
    }
    catch (...)
    {
        return -1;
    }
    return 0;
}
