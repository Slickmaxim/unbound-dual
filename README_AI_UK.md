# Контекст для продовження роботи

Проєкт: gpSP 3D + read-only вкладки PokeDaisy для Unbound 2.1.1.1 EN.
Вихідні дерева й ліцензії описані в THIRD_PARTY_NOTICES.md та AGENTS.md.
Збирати лише PokeDaisy (`make -C 3ds 3ds`), native не збирати.
Картка тренера без значків; пошук оригінальних спрайтів ROM не завершений.
Оригінальні налаштування gpSP та кеш ROM збережені.

Для відтворення помилок потрібна точна CIA та відповідний ELF тієї ж збірки.
Crash dump не публікувати. Нова splash-збірка Luma з невірною версією 0.0.0
може змінювати вибір cache-flush шляху gpSP; ця причина ще не підтверджена
порівнянням на консолі. Не називати це доведеним дефектом.

Based on PokeDaisy by Lidor Itzhari - https://github.com/lidor30/pokedaisy
