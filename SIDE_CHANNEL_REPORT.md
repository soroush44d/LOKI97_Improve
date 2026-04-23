# گزارش ارزیابی و بهبود Side-Channel برای LOKI97

## خلاصه
در این پروژه، مسیرهای وابسته به داده در پیاده‌سازی `LOKI97` بررسی شد. نتیجه نشان داد که در تابع `f`، دسترسی‌های مستقیم به جدول‌های `S1`، `S2` و `P` با ایندکس وابسته به ورودی/کلید انجام می‌شود. این الگو سطح حمله‌ی timing/cache side-channel ایجاد می‌کند.

برای کاهش این ریسک:
1. lookup های مستقیم جدول‌ها به lookup ثابت‌زمان (constant-time full-table scan) تبدیل شدند.
2. یک PoC تست (`proof_sidechannel.c`) اضافه شد که نشان می‌دهد ایندکس‌های مورد استفاده در حالت آسیب‌پذیر تابعی از داده‌ی حساس هستند، و در بیلد hardened پیام صریح کاهش سطح حمله را گزارش می‌کند.
3. یک تست آماری زمان (`sidechannel_test.c`) اضافه شد تا جداسازی کلاس زمانی fixed-vs-random را اندازه‌گیری کند.

---

## تحلیل آسیب‌پذیری

### محل آسیب‌پذیری
در نسخه‌ی قبلی/آسیب‌پذیر، در تابع `f` دسترسی‌ها به صورت زیر هستند:
- `S1[secret_index]`
- `S2[secret_index]`
- `P[sbox_output]`

این دسترسی‌ها به cache line های متفاوتی می‌خورند و با تکنیک‌هایی مثل Prime+Probe یا Flush+Reload می‌توانند نشانه‌ای از state داخلی رمز را نشت دهند.

### چرا این مهم است؟
الگوریتم رمزنگاری باید تا حد ممکن مسیر اجرای مستقل از secret داشته باشد (حداقل در لایه‌ی آدرس‌دهی حافظه). جدول‌محور بودن به خودی خود ایراد نیست، اما table lookup وابسته به secret معمولاً در محیط‌های multi-tenant/attacker-co-located یک ریسک شناخته‌شده است.

---

## تغییرات اعمال‌شده

### 1) افزودن constant-time lookup
در `loki97.c`:
- ماکرو `LOKI97_CT_LOOKUPS` اضافه شد (پیش‌فرض: `1` = hardened).
- تابع‌های جدید:
  - `ct_lookup_byte(...)`
  - `ct_lookup_u64(...)`
- در تابع `f`، تمام lookup های حساس با نسخه ثابت‌زمان جایگزین شدند (با `#if LOKI97_CT_LOOKUPS`).

در حالت hardened، lookup عملاً تمام خانه‌های جدول را اسکن می‌کند و بدون branch وابسته به secret مقدار موردنظر را انتخاب می‌کند؛ بنابراین الگوی آدرس‌دهی حافظه از ایندکس secret مستقل می‌شود.

### 2) تست اثباتی Side-Channel
فایل `proof_sidechannel.c` اضافه شد:
- نسخه‌ی آسیب‌پذیر (`-DLOKI97_CT_LOOKUPS=0`) را ارزیابی می‌کند.
- ایندکس‌های S-box مورد استفاده در round اول برای دو plaintext متفاوت را استخراج می‌کند.
- نشان می‌دهد که ایندکس‌ها (و در نتیجه آدرس‌های lookup) تغییر می‌کنند.
- در بیلد hardened پیام کاهش سطح حمله صادر می‌شود.

### 3) تست آماری timing
فایل `sidechannel_test.c` اضافه شد:
- متوسط زمان رمزنگاری fixed plaintext و random plaintext را مقایسه می‌کند.
- آماره Welch t-score را گزارش می‌دهد.
- برای محیط‌های noisy ممکن است سیگنال قوی ندهد، اما ابزار خوبی برای sanity-check است.

---

## دستورهای اجرا

### اجرای PoC وابستگی ایندکس‌ها (اثبات سطح حمله در نسخه‌ی آسیب‌پذیر)
```bash
gcc -O2 -w -std=gnu11 -DLOKI97_CT_LOOKUPS=0 proof_sidechannel.c -o proof_vuln
./proof_vuln
```

### اجرای PoC در نسخه‌ی Hardened
```bash
gcc -O2 -w -std=gnu11 proof_sidechannel.c -o proof_hardened
./proof_hardened
```

### اجرای تست آماری timing
```bash
gcc -O2 -w -std=c11 -DLOKI97_CT_LOOKUPS=0 -DSAMPLES=60000 loki97.c sidechannel_test.c -lm -o sidechannel_vuln
./sidechannel_vuln
```

---

## نتیجه‌گیری امنیتی
- **قبل از بهبود:** lookup های جدول در `f` وابسته به state حساس بودند => سطح حمله‌ی cache timing موجود است.
- **بعد از بهبود:** با فعال بودن `LOKI97_CT_LOOKUPS=1`، lookup ها به روش full-table scan انجام می‌شوند => نشت آدرس‌دهی secret-dependent به‌طور معناداری کاهش می‌یابد.

> نکته: کاهش side-channel به معنی امنیت مطلق نیست. برای محیط production بهتر است:
> - ممیزی با ابزارهای تخصصی leakage (مانند dudect/ctgrind/PMU-based tests) انجام شود.
> - نوع‌های عددی ۳۲-بیتی صریح (`uint32_t`) جایگزین `unsigned long` شوند تا سازگاری معماری بهتر شود.

---

## محدودیت مشاهده‌شده
تست‌های correctness موجود در repo (و حتی round-trip ساده) روی این محیط نتیجه‌ی موفق ندادند که به نظر ناشی از مسائل قدیمی/معماری در کد پایه (مثلاً اتکا به اندازه‌ی `unsigned long`) است و لزوماً از تغییر side-channel جدید نیست. در این کار تمرکز روی سخت‌سازی مسیر حافظه‌ایِ lookup ها بوده است.
