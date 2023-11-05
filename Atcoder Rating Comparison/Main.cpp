# include <Siv3D.hpp> // Siv3D v0.6.12

struct ContestResult
{
	bool IsRated;
	int32 Place;
	int32 OldRating;
	int32 NewRating;
	int32 Performance;
	int32 InnerPerformance;
	String ContestScreenName;
	String ContestName;
	String ContestNameEn;
	String EndTime;
	Date ContestDate;
};

ContestResult ReadContestResult(const JSON& value)
{
	ContestResult res;
	res.IsRated = value[U"IsRated"].get<bool>();

	res.Place = value[U"Place"].get<int32>();
	res.OldRating = value[U"OldRating"].get<int32>();
	res.NewRating = value[U"NewRating"].get<int32>();
	res.Performance = value[U"Performance"].get<int32>();
	res.InnerPerformance = value[U"InnerPerformance"].get<int32>();
	res.ContestScreenName = value[U"ContestScreenName"].get<String>();
	res.ContestName = value[U"ContestName"].get<String>();
	res.ContestNameEn = value[U"ContestNameEn"].get<String>();
	res.EndTime = value[U"EndTime"].get<String>();

	res.ContestDate.year = ParseOr<int32>(res.EndTime.substr(0, 4), 0);
	res.ContestDate.month = ParseOr<int32>((res.EndTime[5] == '0' ? res.EndTime.substr(6, 1) : res.EndTime.substr(5, 2)), 0);
	res.ContestDate.day = ParseOr<int32>((res.EndTime[8] == '0' ? res.EndTime.substr(9, 1) : res.EndTime.substr(8, 2)), 0);

	return res;
}

Array<ContestResult> ReadUser(const JSON& value)
{
	Array<ContestResult> res;
	for (const auto& element : value.arrayView())
	{
		ContestResult ret = ReadContestResult(element);
		if (ret.IsRated)
		{
			res.push_back(ret);
		}
	}
	return res;
}

Array<JSON> DownloadJsons(const Array<String>& Users)
{
	Array<JSON> res;
	for (auto user : Users)
	{
		URL url = U"https://atcoder.jp/users/" + user + U"/history/json";
		FilePath filepath = U"jsons/" + user + U".json";

		if (SimpleHTTP::Save(url, filepath))
		{
			res.push_back(JSON::Load(filepath));
		}
		else
		{
			throw Error{ U"Failed to download JSON" };
		}
	}
	return res;
}

void Main()
{
	Scene::SetResizeMode(ResizeMode::Actual);
	Window::ResizeActual(800, 600);
	Window::SetTitle(U"Atcoder Rating Comparison");
	Scene::SetBackground(Palette::White);

	Array<Array<ContestResult>> Info;
	Array<JSON> Jsons;

	int32 ratingTop{};

	Date dateBegin;
	Date dateEnd;
	dateBegin.year = 2036;
	dateBegin.month = 2;
	dateBegin.day = 6;
	dateEnd.year = 2000;
	dateEnd.month = 1;
	dateEnd.day = 1;

	TextEditState EditUserName;
	String UserName;
	Array<String> UserNameArr;

	const Array<Color> RatingColor = {
		Color(128,128,128),
		Color(128,64,0),
		Color(0,128,0),
		Color(0,192,192),
		Color(0,0,255),
		Color(192,192,0),
		Color(255,128,0),
		Color(255,0,0),
		Color(255,0,0),
		Color(255,0,0),
		Color(255,0,0),
		Color(255,0,0)
	};

	const Array<Color> UserColor = {
		Palette::Red,
		Palette::Deepskyblue,
		Palette::Limegreen
	};

	while (System::Update())
	{
		SimpleGUI::TextBox(EditUserName, Vec2(10, 10), 690);
		if (SimpleGUI::Button(U"Fetch", Vec2(710, 10), 80))
		{
			Info.clear();
			FileSystem::RemoveContents(U"jsons");
			UserName = EditUserName.text;
			UserNameArr.clear();
			String S;
			for (auto c : UserName)
			{
				if (c == ',')
				{
					UserNameArr.push_back(S);
					S.clear();
				}
				else
				{
					S += c;
				}
			}
			UserNameArr.push_back(S);
			Jsons = DownloadJsons(UserNameArr);

			for (auto& value : Jsons)
			{
				Info.push_back(ReadUser(value));
			}
			dateBegin.year = 2036;
			dateBegin.month = 2;
			dateBegin.day = 6;
			dateEnd.year = 2000;
			dateEnd.month = 1;
			dateEnd.day = 1;
			ratingTop = 0;
		}

		if (ratingTop != 0)
		{
			for (int i : step(ratingTop / 400 + 1))
			{
				Rect(Arg::bottomLeft(50, 400 - 400 * (400 * i) / ratingTop + 150), 700, Min(400 * (400 * (i + 1)) / ratingTop, 400) - 400 * 400 * i / ratingTop).draw(RatingColor[i]);
			}
			Rect(50, 150, 700, 400).draw(ColorF(1, 0.5)).drawFrame(0, 2, Palette::Gray);
		}

		for (int i : step(Info.size()))
		{
			auto& user = Info[i];
			for (int j : step(user.size()))
			{
				int32 period = (dateEnd - dateBegin) / (Days)1;
				double contestDay = (user[j].ContestDate - dateBegin) / (Days)1;
				if (j != 0 && period != 0 && ratingTop != 0)
				{
					Line(680 * contestDay / period + 60, 400 - 400 * user[j].NewRating / ratingTop + 150, 680 * (user[j - 1].ContestDate - dateBegin) / (Days)1 / period + 60, 400 - 400 * user[j - 1].NewRating / ratingTop + 150).draw(2, UserColor[i]);
				}
			}
			for (auto& contest : user)
			{
				if (!contest.IsRated)continue;

				dateBegin = Min(contest.ContestDate, dateBegin);
				dateEnd = Max(contest.ContestDate, dateEnd);
				ratingTop = Max(ratingTop, contest.NewRating + 100);

				int32 period = (dateEnd - dateBegin) / (Days)1;
				double contestDay = (contest.ContestDate - dateBegin) / (Days)1;

				Circle(680 * contestDay / period + 60, 400 - 400 * contest.NewRating / ratingTop + 150, 4).drawShadow(Vec2(0.0, 0.0), 4, 2, UserColor[i]).draw(RatingColor[Min(7, contest.NewRating / 400)]);
			}
		}
	}
}
