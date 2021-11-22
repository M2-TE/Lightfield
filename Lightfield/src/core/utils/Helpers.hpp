#pragma once

#define ROF_DELETE(name)				\
name(const name&) = delete;				\
name(const name&&) = delete;			\
name& operator=(const name&) = delete;	\
name& operator=(const name&&) = delete
