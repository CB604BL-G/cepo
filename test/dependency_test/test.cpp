//A small consumer of an installed CEPO.
//
//It reads its settings from the command line and shows the shape a real project
//takes with cepo::result: each step that can fail returns a result, the caller
//propagates the error with an early return instead of an out-parameter or an
//exception, and the three possible outcomes are reported in one place through
//the ok -> error -> exceptional chain.
//
//Run it against a few inputs and the whole surface is exercised: an ok result,
//an error forwarded up from a step below, a result<void, E> used for a check
//that produces nothing, and the chain deciding which callback to run. With no
//arguments it walks a short sample session, which is what the dependency test
//does, so a successful run is a behavioural check and not just a link check.
#include "cepo/result.hpp"
#include "cepo/result/factory.hpp"

#include <cstdio>
#include <exception>
#include <string>
#include <vector>

namespace
{
	//The error type of this program. An enum keeps the result small and lets the
	//caller choose the message, while the message text stays out of the library.
	enum class config_error
	{
		empty_host,
		missing_value,
		not_a_number,
		out_of_range,
		privileged_port,
		unknown_option
	};

	const char* describe(config_error e) noexcept
	{
		switch (e)
		{
			case config_error::empty_host: return "the host must not be empty";
			case config_error::missing_value: return "an option has no value";
			case config_error::not_a_number: return "the port is not a number";
			case config_error::out_of_range: return "the port is above 65535";
			case config_error::privileged_port: return "the port is below 1024";
			case config_error::unknown_option: return "unknown option";
		}

		return "unknown error";
	}

	struct server_config
	{
		std::string host;
		int port;
	};

	//A single fallible step. Both outcomes leave the function through the
	//factory, which is what lets the result carry the payload or the error
	//without ever being constructed from a bare value.
	cepo::result<int, config_error> parse_port(const std::string& text)
	{
		if (text.empty())
			return cepo::make_result_error<config_error>(config_error::not_a_number);

		int value = 0;

		for (std::string::const_iterator it = text.begin(); it != text.end(); ++it)
		{
			const char c = *it;

			if (c < '0' || c > '9')
				return cepo::make_result_error<config_error>(config_error::not_a_number);

			value = value * 10 + (c - '0');

			if (value > 65535)
				return cepo::make_result_error<config_error>(config_error::out_of_range);
		}

		return cepo::make_result_value<int>(value);
	}

	cepo::result<std::string, config_error> parse_host(const std::string& text)
	{
		if (text.empty())
			return cepo::make_result_error<config_error>(config_error::empty_host);

		return cepo::make_result_value<std::string>(text);
	}

	//A step that checks something but produces nothing uses result<void, E>.
	//The error is carried exactly like before, only the ok side is empty, so the
	//caller reads it with the same if (checked.is_error()) it reads the others.
	cepo::result<void, config_error> validate(const server_config& config)
	{
		if (config.host.empty())
			return cepo::make_result_error<config_error>(config_error::empty_host);

		if (config.port < 1024)
			return cepo::make_result_error<config_error>(config_error::privileged_port);

		return cepo::make_result_value<void>();
	}

	//The caller side of the contract. Every failure below is forwarded with an
	//early return, so the happy path stays flat and no state has to be kept in
	//out-parameters. is_error() is checked first, which is what makes the unwrap
	//that follows total: unwrap_value on an error result would throw
	//bad_result_unwrap instead of returning a reference.
	cepo::result<server_config, config_error> load_config(const std::vector<std::string>& words)
	{
		server_config config;
		config.host = "127.0.0.1";
		config.port = 8080;

		if (words.size() % 2 != 0)
			return cepo::make_result_error<config_error>(config_error::missing_value);

		for (std::size_t i = 0; i < words.size(); i += 2)
		{
			const std::string& option = words[i];
			const std::string& argument = words[i + 1];

			if (option == "--host")
			{
				cepo::result<std::string, config_error> host = parse_host(argument);

				if (host.is_error())
					return cepo::make_result_error<config_error>(host.unwrap_error());

				config.host = host.unwrap_value();
			}
			else if (option == "--port")
			{
				cepo::result<int, config_error> port = parse_port(argument);

				if (port.is_error())
					return cepo::make_result_error<config_error>(port.unwrap_error());

				config.port = port.unwrap_value();
			}
			else
			{
				return cepo::make_result_error<config_error>(config_error::unknown_option);
			}
		}

		cepo::result<void, config_error> checked = validate(config);

		if (checked.is_error())
			return cepo::make_result_error<config_error>(checked.unwrap_error());

		return cepo::make_result_value<server_config>(config);
	}

	//One place reports every outcome. Only the stage that matches the state of
	//the result runs its callback, so there is no if ladder and no way to forget
	//one of the three cases: the chain is nodiscard up to the last stage.
	void report(cepo::result<server_config, config_error>& config)
	{
		config.ok([](server_config& value)
				{
					std::printf("listening on %s:%d\n", value.host.c_str(), value.port);
				})
			.error([](const config_error& e)
				{
					std::printf("config error: %s\n", describe(e));
				})
			.exceptional([](std::exception_ptr e)
				{
					//The third state means a payload threw while it was being
					//moved into the result, so there is no value and no error to
					//report. The recorded exception is rethrown here to say what
					//happened; the library never throws it on its own.
					try
					{
						std::rethrow_exception(e);
					}
					catch (const std::exception& ex)
					{
						std::printf("unexpected failure: %s\n", ex.what());
					}
				});
	}
}

auto main(int argc, char** argv) -> int
{
	if (argc > 1)
	{
		std::vector<std::string> words;

		for (int i = 1; i < argc; ++i)
			words.push_back(argv[i]);

		cepo::result<server_config, config_error> config = load_config(words);

		report(config);

		return config.is_ok() ? 0 : 1;
	}

	//No arguments: walk a short session so the behaviour is visible in the
	//output. Each line is an ordinary call to load_config, exactly as the
	//argument path makes it.
	const std::vector<std::vector<std::string>> samples =
	{
		{ "--host", "example.org", "--port", "9000" },
		{ "--port", "80" },
		{ "--port", "70000" },
		{ "--port", "http" },
		{ "--host", "" },
		{ "--port" },
		{ "--verbose", "on" }
	};

	for (std::size_t i = 0; i < samples.size(); ++i)
	{
		const std::vector<std::string>& words = samples[i];

		std::printf("cepo-demo");

		for (std::size_t w = 0; w < words.size(); ++w)
			std::printf(" %s", words[w].c_str());

		std::printf("\n");

		cepo::result<server_config, config_error> config = load_config(words);

		report(config);
	}

	return 0;
}
