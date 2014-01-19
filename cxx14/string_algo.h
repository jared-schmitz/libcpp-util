#include <string>
#include <vector>

template <typename charT, typename traits> class basic_string_ref;

template <typename charT, typename traits = std::char_traits<charT>>
typename basic_string_ref<charT, traits>::size_type
KMP(basic_string_ref<charT, traits> text,
		basic_string_ref<charT, traits> word)
{
	std::vector<int> T(word.size() + 1, -1);

	// Easy outs
	if (word.size() == 0)
		return 0;
	if (text.size() < word.size())
		return basic_string_ref<charT, traits>::npos;

	// Preprocess
	for (int i = 1; i <= word.size(); i++) {
		int pos = T[i - 1];
		while (pos != -1 && !traits::eq(word[pos], word[i - 1]))
			pos = T[pos];
		T[i] = pos + 1;
	}

	// Search
	int sp = 0;
	int kp = 0;
	while (sp < text.size()) {
		while (kp != -1 && (kp == word.size() || word[kp] != text[sp]))
		       	kp = T[kp];
		kp++;
		sp++;
		if(kp == word.size())
		       	return sp - word.size();
	}

	return basic_string_ref<charT, traits>::npos;
}
