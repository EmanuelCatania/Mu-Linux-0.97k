-- ----------------------------------------------------------------------------
-- Procedimientos de MuOnline97
-- ----------------------------------------------------------------------------
-- Se puede volver a ejecutar: cada procedimiento se borra y se recrea con el
-- charset actual de la base (ver docs/utf8mb4-migracion.md).
USE `MuOnline97`;

-- ----------------------------------------------------------------------------
-- Routine WZ_CONNECT_MEMB
-- ----------------------------------------------------------------------------
DELIMITER $$

DROP PROCEDURE IF EXISTS `WZ_CONNECT_MEMB` $$
CREATE PROCEDURE `WZ_CONNECT_MEMB`(
	IN inAccountID varchar(10),
	IN inServerName varchar(50),
	IN inIpAddress varchar(20)
)
BEGIN
	SET @find_id = 'NOT';
	SET @ConnectStat = 1;

	SELECT S.memb___id INTO @find_id
	FROM `MEMB_STAT` S INNER JOIN `MEMB_INFO` I ON S.memb___id = I.memb___id
	WHERE I.memb___id = `inAccountID`;

	IF (@find_id = 'NOT') THEN
		INSERT INTO `MEMB_STAT` (memb___id, ConnectStat, ServerName, IP, ConnectTM)
		VALUES (`inAccountID`, @ConnectStat, `inServerName`, `inIpAddress`, NOW());
	ELSE
		UPDATE `MEMB_STAT`
		SET ConnectStat = @ConnectStat, ServerName = `inServerName`, IP = `inIpAddress`, ConnectTM = NOW()
		WHERE memb___id = `inAccountID`;
	END IF;
END $$

DELIMITER ;

-- ----------------------------------------------------------------------------
-- Routine WZ_DISCONNECT_MEMB
-- ----------------------------------------------------------------------------
DELIMITER $$

DROP PROCEDURE IF EXISTS `WZ_DISCONNECT_MEMB` $$
CREATE PROCEDURE `WZ_DISCONNECT_MEMB`(
	IN inAccountID VARCHAR(10)
)
BEGIN
	UPDATE `MEMB_STAT`
	SET ConnectStat = 0, DisConnectTM = NOW(), OnlineHours = OnlineHours + TIMESTAMPDIFF(HOUR, ConnectTM, NOW())
	WHERE memb___id = `inAccountID`;
END $$

DELIMITER ;

-- ----------------------------------------------------------------------------
-- Routine WZ_DesblocAccount
-- ----------------------------------------------------------------------------
DELIMITER $$

DROP PROCEDURE IF EXISTS `WZ_DesblocAccount` $$
CREATE PROCEDURE `WZ_DesblocAccount`(
	IN inAccountID VARCHAR(10)
)
BEGIN
	UPDATE `MEMB_INFO`
	SET bloc_code = 0
	WHERE memb___id = `inAccountID` AND bloc_code = 1 AND bloc_expire < NOW();
END $$

DELIMITER ;

-- ----------------------------------------------------------------------------
-- Routine WZ_DesblocCharacters
-- ----------------------------------------------------------------------------
DELIMITER $$

DROP PROCEDURE IF EXISTS `WZ_DesblocCharacters` $$
CREATE PROCEDURE `WZ_DesblocCharacters`(
	IN inAccountID VARCHAR(10)
)
BEGIN
	UPDATE `Character`
	SET CtlCode = 0
	WHERE AccountID = `inAccountID` AND CtlCode = 1 AND bloc_expire < NOW();
END $$

DELIMITER ;

-- ----------------------------------------------------------------------------
-- Routine WZ_GetAccountLevel
-- ----------------------------------------------------------------------------
DELIMITER $$

DROP PROCEDURE IF EXISTS `WZ_GetAccountLevel` $$
CREATE PROCEDURE `WZ_GetAccountLevel` (
	IN inAccountID VARCHAR(10)
)
BEGIN
	SELECT AccountLevel, AccountExpireDate INTO @CurrentAccountLevel, @CurrentAccountExpireDate
	FROM `MEMB_INFO`
	WHERE memb___id = `inAccountID`;

	IF (@CurrentAccountLevel <> 0 AND NOW() >= @CurrentAccountExpireDate) THEN
		SET @CurrentAccountLevel = 0;

		UPDATE `MEMB_INFO`
		SET AccountLevel = 0
		WHERE memb___id = `inAccountID`;
	END IF;

	SELECT @CurrentAccountLevel AS AccountLevel, @CurrentAccountExpireDate AS AccountExpireDate;
END $$

DELIMITER ;

-- ----------------------------------------------------------------------------
-- Routine WZ_SetAccountLevel
-- ----------------------------------------------------------------------------
DELIMITER $$

DROP PROCEDURE IF EXISTS `WZ_SetAccountLevel` $$
CREATE PROCEDURE `WZ_SetAccountLevel`(
	IN inAccountID VARCHAR(10),
	IN inAccountLevel INT,
	IN inAccountExpireTime INT
)
BEGIN
	SELECT AccountLevel, AccountExpireDate INTO @CurrentAccountLevel, @CurrentAccountExpireDate
	FROM `MEMB_INFO`
	WHERE memb___id = `inAccountID`;

	IF (@CurrentAccountLevel = `inAccountLevel`) THEN
		SET @CurrentAccountExpireDate = DATE_ADD(@CurrentAccountExpireDate, INTERVAL `inAccountExpireTime` SECOND);
	ELSE
		SET @CurrentAccountLevel = `inAccountLevel`;
		SET @CurrentAccountExpireDate = DATE_ADD(NOW(), INTERVAL `inAccountExpireTime` SECOND);
	END IF;

	UPDATE `MEMB_INFO`
	SET AccountLevel = @CurrentAccountLevel, AccountExpireDate = @CurrentAccountExpireDate
	WHERE memb___id = `inAccountID`;
END $$

DELIMITER ;

-- ----------------------------------------------------------------------------
-- Routine WZ_CreateCharacter
-- ----------------------------------------------------------------------------
DELIMITER $$

DROP PROCEDURE IF EXISTS `WZ_CreateCharacter` $$
CREATE PROCEDURE `WZ_CreateCharacter`(
	IN inAccountID VARCHAR(10),
	IN inCharName VARCHAR(40),
	IN inCharClass TINYINT
)
BEGIN
	DECLARE EXIT HANDLER FOR SQLEXCEPTION
	BEGIN
		ROLLBACK;
		SELECT 0 AS Result;
	END;

	-- Results cases:
	-- -- Return 1 if success
	-- -- Return 0 will show: 'Invalid character name or name supplied already exists.'
	-- -- Return 2 will show: 'No more characters can be created.'
	SET @Result = 1;

	IF EXISTS (SELECT 1 FROM `Character` WHERE Name = `inCharName`) THEN
		SET @Result = 0;
	ELSE
		START TRANSACTION;
		IF NOT EXISTS (SELECT 1 FROM `AccountCharacter` WHERE Id = `inAccountID`) THEN
			INSERT INTO `AccountCharacter` (Id) VALUES (`inAccountID`);
		ELSEIF NOT EXISTS (
				SELECT 1 FROM `AccountCharacter`
				WHERE Id = `inAccountID`
					AND (GameID1 IS NULL
						OR GameID2 IS NULL
						OR GameID3 IS NULL
						OR GameID4 IS NULL
						OR GameID5 IS NULL)

			) THEN
				SET @Result = 2;
		END IF;

		IF @Result <> 1 THEN
			ROLLBACK;
		ELSE
			INSERT INTO `Character` (AccountID, Name, cLevel, LevelUpPoint, Class, Strength, Dexterity, Vitality, Energy, Inventory, MagicList, Life, MaxLife, Mana, MaxMana, BP, MaxBP, MapNumber, MapPosX, MapPosY, Quest, EffectList)
			SELECT `inAccountID`, `inCharName`, Level, LevelUpPoint, `inCharClass`, Strength, Dexterity, Vitality, Energy, Inventory, MagicList, Life, MaxLife, Mana, MaxMana, 0, 0, MapNumber, MapPosX, MapPosY, Quest, EffectList
			FROM `DefaultClassType` WHERE Class = `inCharClass`;
			COMMIT;
		END IF;
	END IF;

	SELECT @Result AS Result;
END $$

DELIMITER ;

-- ----------------------------------------------------------------------------
-- Routine WZ_DeleteCharacter
-- ----------------------------------------------------------------------------
DELIMITER $$

DROP PROCEDURE IF EXISTS `WZ_DeleteCharacter` $$
CREATE PROCEDURE `WZ_DeleteCharacter`(
	IN inAccountID VARCHAR(10),
	IN inCharName VARCHAR(40)
)
BEGIN
	DECLARE CONTINUE HANDLER FOR SQLEXCEPTION SELECT 1 AS Result;

	-- Results cases:
	-- -- Return 1 if success
	-- -- Return 0 will show: 'You cannot delete a character with guild.'
	-- -- Return 2 will show: 'You have entered an invalid personal ID number.'
	-- -- Return 3 will show: 'This is an item-blocked character.'
	SET @Result = 1;

	IF NOT EXISTS (SELECT 1 FROM `Character` WHERE Name = `inCharName`) THEN
		SET @Result = 0;
	ELSE
		DELETE FROM `Character` WHERE AccountID = `inAccountID` AND Name = `inCharName`;
		DELETE FROM `ResetInfo` WHERE Name = `inCharName`;
		DELETE FROM `OptionData` WHERE Name = `inCharName`;
		DELETE FROM `RankingBloodCastle` WHERE Name = `inCharName`;
		DELETE FROM `RankingDevilSquare` WHERE Name = `inCharName`;

		UPDATE AccountCharacter
			SET GameID1 = NULLIF(GameID1, @inCharName),
				GameID2 = NULLIF(GameID2, @inCharName),
				GameID3 = NULLIF(GameID3, @inCharName),
				GameID4 = NULLIF(GameID4, @inCharName),
				GameID5 = NULLIF(GameID5, @inCharName),
				GameIDC = NULLIF(GameIDC, @inCharName)
			WHERE Id = @inAccountID
			AND (GameID1 = @inCharName
				OR GameID2 = @inCharName
				OR GameID3 = @inCharName
				OR GameID4 = @inCharName
				OR GameID5 = @inCharName
				OR GameIDC = @inCharName);
	END IF;

	SELECT @Result AS Result;
END $$

DELIMITER ;

-- ----------------------------------------------------------------------------
-- Routine WZ_GuildCreate
-- ----------------------------------------------------------------------------
DELIMITER $$

DROP PROCEDURE IF EXISTS `WZ_GuildCreate` $$
CREATE PROCEDURE `WZ_GuildCreate`(
	IN inGuildName VARCHAR(32),
	IN inCharName VARCHAR(40)
)
BEGIN
	DECLARE EXIT HANDLER FOR SQLEXCEPTION
	BEGIN
		ROLLBACK;
		SELECT 6 AS Result;
	END;

	-- Results cases:
	-- -- Return 1 if success
	-- -- Return 0 will show: 'The guild name already exists.'
	-- -- Return 2 will show: 'The guild name must be at least 2 characters long.'
	-- -- Return 3 will show: 'You are already in a guild.'
	-- -- Return 4 will show: 'No space allowed in guild names.'
	-- -- Return 5 will show: 'No symbols allowed in guild names.'
	-- -- Return 6 will show: 'Create guild failed.'
	SET @ErrorCode = 1;

	START TRANSACTION;

	INSERT INTO `Guild` (G_Name, G_Master)
	VALUES (`inGuildName`, `inCharName`);

	IF (ROW_COUNT() = 0) THEN
		SET @ErrorCode = 6;
	ELSE
		INSERT INTO `GuildMember` (Name, G_Name)
		VALUES (`inCharName`, `inGuildName`);

		IF (ROW_COUNT() = 0) THEN
			SET @ErrorCode = 6;
		END IF;
	END IF;

	IF (@ErrorCode <> 1) THEN
		ROLLBACK;
	ELSE
		COMMIT;
	END IF;

	SELECT @ErrorCode as Result;
END $$

DELIMITER ;

-- ----------------------------------------------------------------------------
-- Routine WZ_SetGuildDelete
-- ----------------------------------------------------------------------------
DELIMITER $$

DROP PROCEDURE IF EXISTS `WZ_SetGuildDelete` $$
CREATE PROCEDURE `WZ_SetGuildDelete`(
	IN inGuildName VARCHAR(32)
)
BEGIN
	DECLARE EXIT HANDLER FOR SQLEXCEPTION
	BEGIN
		ROLLBACK;
		SELECT 3 AS Result;
	END;

	-- Results cases:
	-- -- Return 1 if success on member, will show 'You have just left the guild.'
	-- -- Return 4 if success on master, will show: 'The guild has been dissolved'
	-- -- Return 0 will show: 'Your personal ID number is incorrect.'
	-- -- Return 2 will show: 'Only the guild master can disband a guild.'
	-- -- Return 3 will show: 'There was an error deleting the guild'
	SET @Result = 1;

	START TRANSACTION;

	DELETE FROM `GuildMember`
	WHERE G_Name = `inGuildName`;

	IF (ROW_COUNT() = 0) THEN
		SET @Result = 3;
	ELSE
		DELETE FROM `Guild`
		WHERE G_Name = `inGuildName`;

		IF (ROW_COUNT() = 0) THEN
			SET @Result = 3;
		END IF;
	END IF;

	IF (@Result <> 1) THEN
		ROLLBACK;
	ELSE
		COMMIT;
	END IF;

	SELECT @Result AS Result;
END $$

DELIMITER ;

-- ----------------------------------------------------------------------------
-- Routine WZ_GetGrandResetInfo
-- ----------------------------------------------------------------------------
DELIMITER $$

DROP PROCEDURE IF EXISTS `WZ_GetGrandResetInfo` $$
CREATE PROCEDURE `WZ_GetGrandResetInfo` (
	IN inAccountID VARCHAR(10),
	IN inCharName VARCHAR(40)
)
BEGIN
	IF NOT EXISTS (SELECT * FROM `ResetInfo` WHERE Name = `inCharName`) THEN
		INSERT INTO `ResetInfo` (Name) VALUES (`inCharName`);
	END IF;

	SELECT GrandResetCount INTO @GrandReset
	FROM `Character`
	WHERE AccountID = `inAccountID` AND Name = `inCharName`;

	SELECT GrandResetDay, GrandResetDayDate, GrandResetWek, GrandResetWekDate, GrandResetMon, GrandResetMonDate
	INTO @GrandResetDay, @GrandResetDayDate, @GrandResetWek, @GrandResetWekDate, @GrandResetMon, @GrandResetMonDate
	FROM `ResetInfo`
	WHERE Name = `InCharName`;

	IF DATEDIFF(NOW(), @GrandResetDayDate) > 0 THEN
		SET @GrandResetDay = 0;
		UPDATE `ResetInfo` SET GrandResetDay = 0 WHERE Name = `InCharName`;
	END IF;

	IF DATEDIFF(NOW(), @GrandResetWekDate) > 0 THEN
		SET @GrandResetWek = 0;
		UPDATE `ResetInfo` SET GrandResetWek = 0 WHERE Name = `InCharName`;
	END IF;

	IF DATEDIFF(NOW(), @GrandResetMonDate) > 0 THEN
		SET @GrandResetMon = 0;
		UPDATE `ResetInfo` SET GrandResetMon = 0 WHERE Name = `InCharName`;
	END IF;

	SELECT @GrandReset AS GrandReset, @GrandResetDay AS GrandResetDay, @GrandResetWek AS GrandResetWek, @GrandResetMon AS GrandResetMon;
END $$

DELIMITER ;

-- ----------------------------------------------------------------------------
-- Routine WZ_SetGrandResetInfo
-- ----------------------------------------------------------------------------
DELIMITER $$

DROP PROCEDURE IF EXISTS `WZ_SetGrandResetInfo` $$
CREATE PROCEDURE `WZ_SetGrandResetInfo`(
	IN inAccountID VARCHAR(10),
	IN inCharName VARCHAR(40),
	IN inReset INT,
	IN inGrandReset INT,
	IN inGrandResetDay INT,
	IN inGrandResetWek INT,
	IN inGrandResetMon INT
)
BEGIN
	DECLARE EXIT HANDLER FOR SQLEXCEPTION
	BEGIN
		ROLLBACK;
	END;

	START TRANSACTION;

	UPDATE `Character`
	SET ResetCount = `inReset`, GrandResetCount = `inGrandReset`
	WHERE AccountID = `inAccountID` AND Name = `inCharName`;

	UPDATE `ResetInfo`
	SET GrandResetDay = `inGrandResetDay`, GrandResetDayDate = NOW(),
		GrandResetWek = `inGrandResetWek`, GrandResetWekDate = NOW(),
		GrandResetMon = `inGrandResetMon`, GrandResetMonDate = NOW()
	WHERE Name = `inCharName`;

	COMMIT;
END $$

DELIMITER ;

-- ----------------------------------------------------------------------------
-- Routine WZ_GetResetInfo
-- ----------------------------------------------------------------------------
DELIMITER $$

DROP PROCEDURE IF EXISTS `WZ_GetResetInfo` $$
CREATE PROCEDURE `WZ_GetResetInfo`(
	IN inAccountID VARCHAR(10),
	IN inCharName VARCHAR(40)
)
BEGIN
	IF NOT EXISTS (SELECT * FROM `ResetInfo` WHERE Name = `inCharName`) THEN
		INSERT INTO `ResetInfo` (Name) VALUES (`inCharName`);
	END IF;

	SELECT ResetCount INTO @ResetCount
	FROM `Character`
	WHERE AccountID = `inAccountID` AND Name = `inCharName`;

	SELECT ResetDay, ResetDayDate, ResetWek, ResetWekDate, ResetMon, ResetMonDate
	INTO @ResetDay, @ResetDayDate, @ResetWek, @ResetWekDate, @ResetMon, @ResetMonDate
	FROM `ResetInfo`
	WHERE Name = `inCharName`;

	IF DATEDIFF(NOW(), @ResetDayDate) > 0 THEN
		SET @ResetDay = 0;
		UPDATE `ResetInfo` SET ResetDay = 0 WHERE Name = `inCharName`;
	END IF;

	IF DATEDIFF(NOW(), @ResetWekDate) > 0 THEN
		SET @ResetWek = 0;
		UPDATE `ResetInfo` SET ResetWek = 0 WHERE Name = `inCharName`;
	END IF;

	IF DATEDIFF(NOW(), @ResetMonDate) > 0 THEN
		SET @ResetMon = 0;
		UPDATE `ResetInfo` SET ResetMon = 0 WHERE Name = `inCharName`;
	END IF;

	SELECT @ResetCount AS Reset, @ResetDay AS ResetDay, @ResetWek AS ResetWek, @ResetMon AS ResetMon;
END $$

DELIMITER ;

-- ----------------------------------------------------------------------------
-- Routine WZ_SetResetInfo
-- ----------------------------------------------------------------------------
DELIMITER $$

DROP PROCEDURE IF EXISTS `WZ_SetResetInfo` $$
CREATE PROCEDURE `WZ_SetResetInfo`(
	IN inAccountID VARCHAR(10),
	IN inCharName VARCHAR(40),
	IN inReset INT,
	IN inResetDay INT,
	IN inResetWek INT,
	IN inResetMon INT
)
BEGIN
	DECLARE EXIT HANDLER FOR SQLEXCEPTION
	BEGIN
		ROLLBACK;
	END;

	START TRANSACTION;

	UPDATE `Character`
	SET ResetCount = `inReset`
	WHERE AccountID = `inAccountID` AND Name = `inCharName`;

	UPDATE `ResetInfo`
	SET ResetDay = `inResetDay`, ResetDayDate = NOW(),
		ResetWek = `inResetWek`, ResetWekDate = NOW(),
		ResetMon = `inResetMon`, ResetMonDate = NOW()
	WHERE Name = `inCharName`;

	COMMIT;
END $$

DELIMITER ;

-- ----------------------------------------------------------------------------
-- Routine WZ_GetItemSerial
-- ----------------------------------------------------------------------------
DELIMITER $$

DROP PROCEDURE IF EXISTS `WZ_GetItemSerial` $$
CREATE PROCEDURE `WZ_GetItemSerial`()
BEGIN
	DECLARE EXIT HANDLER FOR SQLEXCEPTION
	BEGIN
		ROLLBACK;
		SELECT -1 as Result;
	END;

	START TRANSACTION;

	SELECT ItemCount INTO @ItemSerial
	FROM `GameServerInfo`
	WHERE Number = 0;

	IF (@ItemSerial >= 2147483647) THEN -- MAX INTEGER VALUE
		SET @ItemSerial = 1;
	ELSE
		SET @ItemSerial = @ItemSerial + 1;
	END IF;

	UPDATE `GameServerInfo`
	SET ItemCount = @ItemSerial
	WHERE Number = 0;

	IF (ROW_COUNT() = 0) THEN
		ROLLBACK;
		SELECT -1 as Result;
	ELSE
		COMMIT;
		SELECT @ItemSerial as Result;
	END IF;
END $$

DELIMITER ;
